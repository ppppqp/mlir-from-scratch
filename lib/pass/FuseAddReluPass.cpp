#include "mini/pass/FuseAddReluPass.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/MiniGenericOp.h"
namespace mini::pass {

bool FuseAddReluPass::run(ir::Operation& operation, std::string& error) {
  error.clear();
  if (operation.name() != "func.func") {
    error = "fuse-add-relu expects func.func";
    return false;
  }

  ir::Block& block = ir::FuncOp(operation).entryBlock();
  // TODO(lesson 6.5): find mini.relu whose operand is the result of a
  // mini.add in this same block. Require the add result to have exactly one
  // use, owned by that relu at operand position 0.
  //
  // Build mini.generic(A, B) -> tensor before the add. Its region has two f32
  // block arguments and scalar_add -> scalar_relu -> yield. Verify the new
  // generic before changing the old IR.
  //
  // RAUW the relu result with the generic result, erase relu, then erase add.
  // Keep traversal indices valid across insertion and both erasures.
  while (true) {
    bool rewritten = false;
    for (std::size_t index = 0; index < block.operations().size(); ++index) {
      ir::Operation& add = *block.operations()[index];
      if (add.name() != "mini.add" || add.operands().size() != 2 ||
          add.resultCount() != 1) {
        continue;
      }
      ir::Value& sum = add.result(0);
      if (sum.useCount() != 1) continue;
      const ir::Use use = sum.uses()[0];
      ir::Operation* relu = use.owner;
      if (use.operandIndex != 0 || relu->name() != "mini.relu" ||
          relu->operands().size() != 1 || relu->resultCount() != 1) {
        continue;
      }
      // The producer must precede its user in this same block.
      bool inBlockAfterAdd = false;
      for (std::size_t later = index + 1; later < block.operations().size();
           ++later) {
        if (block.operations()[later].get() == relu) {
          inBlockAfterAdd = true;
          break;
        }
      }
      if (!inBlockAfterAdd) continue;

      auto generic = std::make_unique<ir::Operation>(
          "mini.generic",
          std::vector<ir::Value*>{add.operands()[0], add.operands()[1]},
          std::vector<ir::Type>{relu->result(0).type()});
      ir::Block& body = generic->addRegion().addBlock();
      ir::Value& scalarA = body.addArgument(ir::Type::f32());
      ir::Value& scalarB = body.addArgument(ir::Type::f32());
      ir::Operation& scalarAdd = body.append(std::make_unique<ir::Operation>(
          "mini.scalar_add", std::vector<ir::Value*>{&scalarA, &scalarB},
          std::vector<ir::Type>{ir::Type::f32()}));
      ir::Operation& scalarRelu = body.append(std::make_unique<ir::Operation>(
          "mini.scalar_relu", std::vector<ir::Value*>{&scalarAdd.result(0)},
          std::vector<ir::Type>{ir::Type::f32()}));
      body.append(std::make_unique<ir::Operation>(
          "mini.yield", std::vector<ir::Value*>{&scalarRelu.result(0)},
          std::vector<ir::Type>{}));

      std::string genericError;
      if (!ir::verifyOperation(*generic, genericError)) {
        error = "cannot fuse add and relu: " + genericError;
        return false;
      }
      ir::Operation& fused = block.insertBefore(add, std::move(generic));
      relu->result(0).replaceAllUsesWith(fused.result(0));
      block.erase(*relu);
      block.erase(add);
      rewritten = true;
      break;
    }
    if (!rewritten) break;
  }
  return true;
}

}  // namespace mini::pass
