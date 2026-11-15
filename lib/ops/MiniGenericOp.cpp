#include "mini/ops/MiniGenericOp.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/MiniScalarAddOp.h"
#include "mini/ops/MiniScalarReluOp.h"
#include "mini/ops/MiniYieldOp.h"
namespace mini::ops {

MiniGenericOp::MiniGenericOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected mini.generic operation");
  }
}

bool MiniGenericOp::verify(std::string& error) const {
  error.clear();
  // TODO(lesson 6.4): check two equally shaped tensor<f32> operands, one
  // matching tensor result, one region with one block, and two f32 block args.
  // Verify the scalar operations in SSA order, and require a final mini.yield
  // of one f32 value defined in the block. Reject nested regions and unknown
  // scalar operations. Use ir::verifyOperation for scalar op checks.
  const auto operands = operation_->operands();
  if (operands.size() != 2 || operation_->resultCount() != 1) {
    error = "mini.generic requires two operands and one result";
    return false;
  }
  const ir::Type& tensorType = operands[0]->type();
  if (tensorType.kind() != ir::Type::Kind::Tensor ||
      tensorType.shape().empty() ||
      std::any_of(tensorType.shape().begin(), tensorType.shape().end(),
                  [](auto dimension) { return dimension <= 0; }) ||
      operands[1]->type() != tensorType ||
      operation_->result(0).type() != tensorType) {
    error = "mini.generic operands and result must have the same nonempty, positive tensor shape";
    return false;
  }
  if (operation_->regions().size() != 1 ||
      operation_->regions()[0]->blocks().size() != 1) {
    error = "mini.generic requires one region with one block";
    return false;
  }

  const ir::Block& block = *operation_->regions()[0]->blocks()[0];
  const auto arguments = block.arguments();
  if (arguments.size() != 2 || arguments[0]->type() != ir::Type::f32() ||
      arguments[1]->type() != ir::Type::f32()) {
    error = "mini.generic block requires two f32 arguments";
    return false;
  }
  const auto operations = block.operations();
  if (operations.empty() || operations.back()->name() != MiniYieldOp::name) {
    error = "mini.generic block must end with mini.yield";
    return false;
  }

  std::unordered_set<const ir::Value*> available{arguments[0].get(),
                                                 arguments[1].get()};
  for (std::size_t index = 0; index < operations.size(); ++index) {
    const ir::Operation& child = *operations[index];
    if (child.name() != MiniScalarAddOp::name &&
        child.name() != MiniScalarReluOp::name &&
        child.name() != MiniYieldOp::name) {
      error = "mini.generic contains unsupported scalar operation";
      return false;
    }
    if (child.name() == MiniYieldOp::name && index + 1 != operations.size()) {
      error = "mini.yield must be the final operation";
      return false;
    }
    for (const ir::Value* operand : child.operands()) {
      if (!available.contains(operand)) {
        error = "mini.generic scalar operand is outside the block or used before its definition";
        return false;
      }
    }
    std::string childError;
    if (!ir::verifyOperation(child, childError)) {
      error = "mini.generic operation #" + std::to_string(index) + ": " +
              childError;
      return false;
    }
    for (std::size_t result = 0; result < child.resultCount(); ++result) {
      available.insert(&child.result(result));
    }
  }

  return true;
}

}  // namespace mini::ops
