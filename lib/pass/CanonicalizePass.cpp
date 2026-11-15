#include "mini/pass/CanonicalizePass.h"

#include <cstddef>
#include <string>

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
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

  return true;
}

}  // namespace mini::pass
