#include "mini/interpreter/Interpreter.h"

#include <algorithm>
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
  std::unordered_map<const ir::Value*, TensorData> values;
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
        operation.name() != ops::MiniGenericOp::name) {
      throw std::invalid_argument("unsupported interpreter operation: " +
                                  std::string(operation.name()));
    }
    std::string error;
    if (!ir::verifyOperation(operation, error)) {
      throw std::invalid_argument(error);
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
