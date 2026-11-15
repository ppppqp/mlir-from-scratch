#include "mini/analysis/UseCountAnalysis.h"

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"

#include <stdexcept>

namespace mini::analysis {

UseCountAnalysis::UseCountAnalysis(const ir::Operation& function) {
  if (function.name() != "func.func" || function.regions().size() != 1 ||
      function.regions()[0]->blocks().size() != 1) {
    throw std::invalid_argument(
        "UseCountAnalysis requires a func.func with one region and one block");
  }

  const ir::Block& block = *function.regions()[0]->blocks()[0];
  for (const auto& argument : block.arguments()) {
    counts_.emplace(argument.get(), 0);
  }
  for (const auto& operation : block.operations()) {
    for (std::size_t i = 0; i < operation->resultCount(); ++i) {
      counts_.emplace(&operation->result(i), 0);
    }
  }
  for (const auto& operation : block.operations()) {
    for (const ir::Value* operand : operation->operands()) {
      const auto it = counts_.find(operand);
      if (it == counts_.end()) {
        throw std::invalid_argument(
            "operand is not defined in this function's entry block");
      }
      ++it->second;
    }
  }
}

std::optional<std::size_t> UseCountAnalysis::useCount(
    const ir::Value& value) const {
  const auto it = counts_.find(&value);
  if (it == counts_.end()) {
    return std::nullopt;
  }
  return it->second;
}

}  // namespace mini::analysis
