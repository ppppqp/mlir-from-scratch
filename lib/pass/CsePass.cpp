#include "mini/pass/CsePass.h"

#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
#include "mini/ops/SideEffectInterface.h"
namespace mini::pass {
namespace {

bool isValueListMatch(std::span<ir::Value* const> lhs,
                      std::span<ir::Value* const> rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

bool isSupported(std::string_view name) {
  return name == "mini.relu" || name == "mini.add" ||
         name == "mini.matmul";
}

bool resultTypesMatch(const ir::Operation& lhs, const ir::Operation& rhs) {
  if (lhs.resultCount() != rhs.resultCount()) return false;
  for (std::size_t i = 0; i < lhs.resultCount(); ++i) {
    if (lhs.result(i).type() != rhs.result(i).type()) return false;
  }
  return true;
}

}  // namespace

bool CsePass::run(ir::Operation& operation, std::string& error) {
  error.clear();
  if (operation.name() != "func.func") {
    error = "cse expects func.func";
    return false;
  }

  ir::Block& block = ir::FuncOp(operation).entryBlock();
  // TODO(lesson 6.3): visit operations from front to back. For each supported
  // operation, find an earlier equivalent operation in this block:
  //   - both have no side effects (query SideEffectInterface);
  //   - names and ordered operand Value* lists match;
  //   - result counts and each corresponding result type match.
  // The supported operations are mini.relu, mini.add, and mini.matmul.
  // Replace every result use of the later operation with the matching earlier
  // result, then erase the later operation. Keep indices valid after erase.
  for (std::size_t index = 0; index < block.operations().size();) {
    ir::Operation& current = *block.operations()[index];
    if (!isSupported(current.name()) ||
        ops::SideEffectInterface::hasSideEffects(current)) {
      ++index;
      continue;
    }

    bool erased = false;
    for (std::size_t i = 0; i < index; ++i) {
      ir::Operation& previous = *block.operations()[i];
      if (ops::SideEffectInterface::hasSideEffects(previous) ||
          previous.name() != current.name() ||
          !isValueListMatch(previous.operands(), current.operands()) ||
          !resultTypesMatch(previous, current)) {
        continue;
      }
      for (std::size_t j = 0; j < current.resultCount(); ++j) {
        current.result(j).replaceAllUsesWith(previous.result(j));
      }
      block.erase(current);
      erased = true;
      break;
    }
    if (!erased) ++index;
  }
  return true;
}

}  // namespace mini::pass
