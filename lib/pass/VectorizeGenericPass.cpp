#include "mini/pass/VectorizeGenericPass.h"

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/VectorOps.h"

#include <string>

namespace mini::pass {

bool VectorizeGenericPass::run(ir::Operation& operation, std::string& error) {
  error.clear();
  if (operation.name() != "func.func") {
    error = "vectorize-generic expects func.func";
    return false;
  }

  ir::Block& functionBody = ir::FuncOp(operation).entryBlock();
  // TODO(lesson 7.3): visit scf.for bodies and find mini.generic on two
  // tensor<4xf32> inputs. Match a region with scalar_add -> scalar_relu ->
  // mini.yield. Insert vector.from_tensor for each input, vector.add,
  // vector.relu, and vector.to_tensor before the generic. RAUW its result
  // with vector.to_tensor's result, then erase the generic. Keep the loop's
  // tensor.insert_slice and scf.yield; they handle the tail tile mask.
  // Verify the new operations before replacing any uses.
  (void)functionBody;
  return true;
}

}  // namespace mini::pass
