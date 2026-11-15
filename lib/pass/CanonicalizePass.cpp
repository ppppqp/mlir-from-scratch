#include "mini/pass/CanonicalizePass.h"

#include <cstddef>
#include <string>

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
#include "mini/ops/SideEffectInterface.h"
namespace mini::pass {

bool CanonicalizePass::run(ir::Operation& operation, std::string& error) {
  error.clear();
  if (operation.name() != "func.func") {
    error = "canonicalize expects func.func";
    return false;
  }

  ir::Block& block = ir::FuncOp(operation).entryBlock();
  for (std::size_t index = 0; index < block.operations().size();) {
    ir::Operation& outer = *block.operations()[index];
    if (outer.name() != "mini.relu" || outer.operands().size() != 1 ||
        outer.resultCount() != 1) {
      ++index;
      continue;
    }

    ir::Value* innerResult = outer.operands()[0];
    ir::Operation* inner = innerResult->definingOp();
    if (inner == nullptr || inner->name() != "mini.relu") {
      ++index;
      continue;
    }

    outer.result(0).replaceAllUsesWith(*innerResult);
    block.erase(outer);
    // Erase shifts the next operation into this index.
  }

  // A deleted user may make its producer dead before the producer is visited.
  for (std::size_t remaining = block.operations().size(); remaining > 0;) {
    ir::Operation& current = *block.operations()[--remaining];
    if (ops::SideEffectInterface::hasSideEffects(current)) {
      continue;
    }
    bool dead = true;
    for (std::size_t i = 0; i < current.resultCount(); ++i) {
      if (current.result(i).useCount() > 0) {
        dead = false;
        break;
      }
    }
    if (dead) {
      block.erase(current);
    }
  }

  return true;
}

}  // namespace mini::pass
