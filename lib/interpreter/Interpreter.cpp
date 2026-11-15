#include "mini/interpreter/Interpreter.h"

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace mini::interpreter {
namespace {

std::size_t elementCount(const ir::Type& type) {
  if (type.kind() != ir::Type::Kind::Tensor || type.shape().empty()) {
    throw std::invalid_argument("interpreter expects a tensor type");
  }
  std::size_t count = 1;
  for (const std::int64_t dimension : type.shape()) {
    if (dimension <= 0 ||
        static_cast<std::uint64_t>(dimension) >
            std::numeric_limits<std::size_t>::max() / count) {
      throw std::invalid_argument("invalid tensor element count");
    }
    count *= static_cast<std::size_t>(dimension);
  }
  return count;
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
    throw std::invalid_argument("function input count does not match arguments");
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
    if (operation.name() != "mini.add" && operation.name() != "mini.relu") {
      throw std::invalid_argument("unsupported interpreter operation: " +
                                  std::string(operation.name()));
    }
    std::string error;
    if (!ir::verifyOperation(operation, error)) {
      throw std::invalid_argument(error);
    }

    const TensorData& input = lookup(operation.operands()[0]);
    std::vector<float> result(input.elements.size());
    if (operation.name() == "mini.add") {
      const TensorData& rhs = lookup(operation.operands()[1]);
      for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = input.elements[i] + rhs.elements[i];
      }
    } else {
      for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = std::max(input.elements[i], 0.0f);
      }
    }
    values.emplace(&operation.result(0),
                   TensorData{operation.result(0).type(), std::move(result)});
  }
  throw std::invalid_argument("function body has no func.return");
}

}  // namespace mini::interpreter
