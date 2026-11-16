#include "mini/pass/VectorizeGenericPass.h"

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
#include "mini/ops/MiniScalarAddOp.h"
#include "mini/ops/MiniScalarReluOp.h"
#include "mini/ops/MiniYieldOp.h"
#include "mini/ops/TilingOps.h"
#include "mini/ops/VectorOps.h"

namespace mini::pass {
namespace {

bool matchesAddRelu(const ir::Operation& generic) {
  if (generic.operands().size() != 2 || generic.resultCount() != 1 ||
      generic.operands()[0]->type() != ir::Type::tensor({4}) ||
      generic.operands()[1]->type() != ir::Type::tensor({4}) ||
      generic.result(0).type() != ir::Type::tensor({4}) ||
      generic.regions().size() != 1 ||
      generic.regions()[0]->blocks().size() != 1) {
    return false;
  }
  const ir::Block& body = *generic.regions()[0]->blocks()[0];
  if (body.arguments().size() != 2 || body.operations().size() != 3) {
    return false;
  }
  const ir::Operation& add = *body.operations()[0];
  const ir::Operation& relu = *body.operations()[1];
  const ir::Operation& yield = *body.operations()[2];
  return add.name() == ops::MiniScalarAddOp::name &&
         add.operands().size() == 2 && add.resultCount() == 1 &&
         add.operands()[0] == body.arguments()[0].get() &&
         add.operands()[1] == body.arguments()[1].get() &&
         relu.name() == ops::MiniScalarReluOp::name &&
         relu.operands().size() == 1 && relu.resultCount() == 1 &&
         relu.operands()[0] == &add.result(0) &&
         yield.name() == ops::MiniYieldOp::name &&
         yield.operands().size() == 1 && yield.resultCount() == 0 &&
         yield.operands()[0] == &relu.result(0);
}

}  // namespace

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
  for (const auto& topLevel : functionBody.operations()) {
    if (topLevel->name() != ops::ScfForOp::name) continue;
    std::string verifyError;
    if (!ir::verifyOperation(*topLevel, verifyError)) {
      error = "cannot vectorize invalid scf.for: " + verifyError;
      return false;
    }
    ir::Block& loopBody = *topLevel->regions()[0]->blocks()[0];
    while (true) {
      bool rewritten = false;
      for (const auto& child : loopBody.operations()) {
        if (child->name() != ops::MiniGenericOp::name ||
            !matchesAddRelu(*child)) {
          continue;
        }
        ir::Operation& generic = *child;
        if (!ir::verifyOperation(generic, verifyError)) {
          error = "cannot vectorize invalid mini.generic: " + verifyError;
          return false;
        }

        const ir::Type vectorType = ir::Type::vector({4});
        const ir::Type tensorType = ir::Type::tensor({4});
        auto lhs = std::make_unique<ir::Operation>(
            std::string(ops::VectorFromTensorOp::name),
            std::vector<ir::Value*>{generic.operands()[0]},
            std::vector<ir::Type>{vectorType});
        auto rhs = std::make_unique<ir::Operation>(
            std::string(ops::VectorFromTensorOp::name),
            std::vector<ir::Value*>{generic.operands()[1]},
            std::vector<ir::Type>{vectorType});
        auto sum = std::make_unique<ir::Operation>(
            std::string(ops::VectorAddOp::name),
            std::vector<ir::Value*>{&lhs->result(0), &rhs->result(0)},
            std::vector<ir::Type>{vectorType});
        auto relu = std::make_unique<ir::Operation>(
            std::string(ops::VectorReluOp::name),
            std::vector<ir::Value*>{&sum->result(0)},
            std::vector<ir::Type>{vectorType});
        auto result = std::make_unique<ir::Operation>(
            std::string(ops::VectorToTensorOp::name),
            std::vector<ir::Value*>{&relu->result(0)},
            std::vector<ir::Type>{tensorType});
        if (!ir::verifyOperation(*lhs, verifyError) ||
            !ir::verifyOperation(*rhs, verifyError) ||
            !ir::verifyOperation(*sum, verifyError) ||
            !ir::verifyOperation(*relu, verifyError) ||
            !ir::verifyOperation(*result, verifyError)) {
          error = "constructed invalid vector IR: " + verifyError;
          return false;
        }

        loopBody.insertBefore(generic, std::move(lhs));
        loopBody.insertBefore(generic, std::move(rhs));
        loopBody.insertBefore(generic, std::move(sum));
        loopBody.insertBefore(generic, std::move(relu));
        ir::Operation& converted =
            loopBody.insertBefore(generic, std::move(result));
        generic.result(0).replaceAllUsesWith(converted.result(0));
        loopBody.erase(generic);
        rewritten = true;
        break;
      }
      if (!rewritten) break;
    }
  }
  return true;
}

}  // namespace mini::pass
