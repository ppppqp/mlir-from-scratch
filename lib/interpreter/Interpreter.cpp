#include "mini/interpreter/Interpreter.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/MiniDpsAddOp.h"
#include "mini/ops/MiniDpsReluOp.h"
#include "mini/ops/MiniGenericOp.h"
#include "mini/ops/TensorEmptyOp.h"
#include "mini/ops/TilingOps.h"

namespace mini::interpreter {
namespace {

std::size_t elementCount(const ir::Type& type) {
  if (type.kind() != ir::Type::Kind::Tensor || type.shape().empty()) {
    throw std::invalid_argument("interpreter expects a tensor type");
  }
  std::size_t count = 1;
  for (const std::int64_t dimension : type.shape()) {
    if (dimension <= 0 || static_cast<std::uint64_t>(dimension) >
                              std::numeric_limits<std::size_t>::max() / count) {
      throw std::invalid_argument("invalid tensor element count");
    }
    count *= static_cast<std::size_t>(dimension);
  }
  return count;
}

TensorData evaluateGeneric(const ir::Operation& generic,
                           const TensorData& lhs, const TensorData& rhs) {
  const ir::Block& body = *generic.regions()[0]->blocks()[0];
  const auto arguments = body.arguments();
  std::vector<float> result(elementCount(generic.result(0).type()));

  for (std::size_t element = 0; element < result.size(); ++element) {
    // Scalar SSA values belong to this element's region invocation.
    std::unordered_map<const ir::Value*, float> scalars;
    scalars.emplace(arguments[0].get(), lhs.elements[element]);
    scalars.emplace(arguments[1].get(), rhs.elements[element]);
    const auto lookupScalar = [&scalars](const ir::Value* value) -> float {
      const auto it = scalars.find(value);
      if (it == scalars.end()) {
        throw std::invalid_argument("use of unavailable scalar SSA value");
      }
      return it->second;
    };

    for (const auto& child : body.operations()) {
      if (child->name() == "mini.scalar_add") {
        scalars.emplace(&child->result(0),
                        lookupScalar(child->operands()[0]) +
                            lookupScalar(child->operands()[1]));
      } else if (child->name() == "mini.scalar_relu") {
        scalars.emplace(&child->result(0),
                        std::max(lookupScalar(child->operands()[0]), 0.0f));
      } else if (child->name() == "mini.yield") {
        result[element] = lookupScalar(child->operands()[0]);
        break;
      } else {
        throw std::invalid_argument("unsupported scalar operation in mini.generic");
      }
    }
  }
  return TensorData{generic.result(0).type(), std::move(result)};
}

using TensorValues = std::unordered_map<const ir::Value*, TensorData>;

TensorData evaluateLoop(const ir::Operation& loop, const TensorValues& outer) {
  const auto bound = [&loop](std::string_view key) -> std::int64_t {
    return std::get<std::int64_t>(*loop.findAttribute(key));
  };
  const std::int64_t lower = bound("lower_bound");
  const std::int64_t upper = bound("upper_bound");
  const std::int64_t step = bound("step");
  const ir::Block& body = *loop.regions()[0]->blocks()[0];
  TensorData carried = outer.at(loop.operands()[0]);

  for (std::int64_t iv = lower; iv < upper;) {
    TensorValues local;
    local.emplace(body.arguments()[1].get(), carried);
    const auto lookupTensor = [&local, &outer](const ir::Value* value)
        -> const TensorData& {
      if (const auto it = local.find(value); it != local.end()) return it->second;
      if (const auto it = outer.find(value); it != outer.end()) return it->second;
      throw std::invalid_argument("use of unavailable tensor SSA value in scf.for");
    };
    const auto lookupIndex = [&body, iv](const ir::Value* value) -> std::size_t {
      if (value != body.arguments()[0].get() || iv < 0) {
        throw std::invalid_argument("use of unavailable index SSA value in scf.for");
      }
      return static_cast<std::size_t>(iv);
    };

    for (const auto& child : body.operations()) {
      if (child->name() == ops::TensorExtractSliceOp::name) {
        const TensorData& source = lookupTensor(child->operands()[0]);
        const std::size_t offset = lookupIndex(child->operands()[1]);
        if (offset >= source.elements.size()) {
          throw std::invalid_argument("tensor.extract_slice offset out of bounds");
        }
        std::vector<float> tile(elementCount(child->result(0).type()), 0.0f);
        const std::size_t valid = std::min(tile.size(), source.elements.size() - offset);
        std::copy_n(source.elements.begin() + offset, valid, tile.begin());
        local.emplace(&child->result(0),
                      TensorData{child->result(0).type(), std::move(tile)});
      } else if (child->name() == ops::MiniGenericOp::name) {
        local.emplace(&child->result(0),
                      evaluateGeneric(*child, lookupTensor(child->operands()[0]),
                                      lookupTensor(child->operands()[1])));
      } else if (child->name() == ops::TensorInsertSliceOp::name) {
        const TensorData& tile = lookupTensor(child->operands()[0]);
        TensorData updated = lookupTensor(child->operands()[1]);
        const std::size_t offset = lookupIndex(child->operands()[2]);
        if (offset >= updated.elements.size()) {
          throw std::invalid_argument("tensor.insert_slice offset out of bounds");
        }
        const std::size_t valid =
            std::min(tile.elements.size(), updated.elements.size() - offset);
        std::copy_n(tile.elements.begin(), valid, updated.elements.begin() + offset);
        local.emplace(&child->result(0), std::move(updated));
      } else if (child->name() == ops::ScfYieldOp::name) {
        carried = lookupTensor(child->operands()[0]);
      } else {
        throw std::invalid_argument("unsupported operation in scf.for body");
      }
    }
    if (upper - iv <= step) break;
    iv += step;
  }
  return carried;
}

}  // namespace

TensorData runFunction(const ir::Operation& function,
                       std::span<const TensorData> inputs) {
  if (function.name() != "func.func" || function.regions().size() != 1 ||
      function.regions()[0]->blocks().size() != 1) {
    throw std::invalid_argument(
        "interpreter requires func.func with one region and one block");
  }
  const ir::Block& block = *function.regions()[0]->blocks()[0];
  if (inputs.size() != block.arguments().size()) {
    throw std::invalid_argument(
        "function input count does not match arguments");
  }
  TensorValues values;
  for (std::size_t i = 0; i < inputs.size(); ++i) {
    if (inputs[i].type != block.arguments()[i]->type() ||
        inputs[i].elements.size() != elementCount(inputs[i].type)) {
      throw std::invalid_argument("function input type or size mismatch");
    }
    values.emplace(block.arguments()[i].get(), inputs[i]);
  }

  const auto lookup = [&values](const ir::Value* value) -> const TensorData& {
    const auto it = values.find(value);
    if (it == values.end()) {
      throw std::invalid_argument("use of unavailable SSA value");
    }
    return it->second;
  };

  for (std::size_t index = 0; index < block.operations().size(); ++index) {
    const ir::Operation& operation = *block.operations()[index];
    if (operation.name() == "func.return") {
      if (index + 1 != block.operations().size() ||
          operation.operands().size() != 1 || operation.resultCount() != 0) {
        throw std::invalid_argument("invalid func.return");
      }
      return lookup(operation.operands()[0]);
    }
    const bool isAdd = operation.name() == "mini.add" ||
                       operation.name() == ops::MiniDpsAddOp::name;
    const bool isRelu = operation.name() == "mini.relu" ||
                        operation.name() == ops::MiniDpsReluOp::name;
    if (!isAdd && !isRelu && operation.name() != "mini.matmul" &&
        operation.name() != ops::MiniGenericOp::name &&
        operation.name() != ops::TensorEmptyOp::name &&
        operation.name() != ops::ScfForOp::name) {
      throw std::invalid_argument("unsupported interpreter operation: " +
                                  std::string(operation.name()));
    }
    std::string error;
    if (!ir::verifyOperation(operation, error)) {
      throw std::invalid_argument(error);
    }

    if (operation.name() == ops::TensorEmptyOp::name) {
      values.emplace(&operation.result(0),
                     TensorData{operation.result(0).type(),
                                std::vector<float>(
                                    elementCount(operation.result(0).type()),
                                    std::numeric_limits<float>::quiet_NaN())});
      continue;
    }
    if (operation.name() == ops::ScfForOp::name) {
      values.emplace(&operation.result(0), evaluateLoop(operation, values));
      continue;
    }

    if (operation.name() == ops::MiniGenericOp::name) {
      values.emplace(&operation.result(0),
                     evaluateGeneric(operation, lookup(operation.operands()[0]),
                                     lookup(operation.operands()[1])));
      continue;
    }

    if (operation.name() == ops::MiniDpsAddOp::name) {
      (void)lookup(operation.operands()[ops::MiniDpsAddOp::outputOperandIndex]);
    } else if (operation.name() == ops::MiniDpsReluOp::name) {
      (void)lookup(operation.operands()[ops::MiniDpsReluOp::outputOperandIndex]);
    }

    const TensorData& input = lookup(operation.operands()[0]);
    std::vector<float> result(elementCount(operation.result(0).type()));
    if (isAdd) {
      const TensorData& rhs = lookup(operation.operands()[1]);
      for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = input.elements[i] + rhs.elements[i];
      }
    } else if (isRelu) {
      for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = std::max(input.elements[i], 0.0f);
      }
    } else {
      const TensorData& rhs = lookup(operation.operands()[1]);
      const auto lhsShape = input.type.shape();
      const auto rhsShape = rhs.type.shape();
      const auto mSize = static_cast<std::size_t>(lhsShape[0]);
      const auto kSize = static_cast<std::size_t>(lhsShape[1]);
      const auto nSize = static_cast<std::size_t>(rhsShape[1]);
      for (std::size_t m = 0; m < mSize; ++m) {
        for (std::size_t n = 0; n < nSize; ++n) {
          float sum = 0.0f;
          for (std::size_t k = 0; k < kSize; ++k) {
            sum += input.elements[m * kSize + k] * rhs.elements[k * nSize + n];
          }
          result[m * nSize + n] = sum;
        }
      }
    }
    values.emplace(&operation.result(0),
                   TensorData{operation.result(0).type(), std::move(result)});
  }
  throw std::invalid_argument("function body has no func.return");
}

}  // namespace mini::interpreter
