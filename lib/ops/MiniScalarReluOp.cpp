#include "mini/ops/MiniScalarReluOp.h"

#include <stdexcept>

#include "mini/ir/Operation.h"

namespace mini::ops {

MiniScalarReluOp::MiniScalarReluOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected mini.scalar_relu operation");
  }
}

bool MiniScalarReluOp::verify(std::string& error) const {
  error.clear();
  // TODO(lesson 6.4): require one f32 operand, one f32 result, no regions.
  if (operation_->operands().size() != 1 || operation_->resultCount() != 1 ||
      !operation_->regions().empty() ||
      operation_->operands()[0]->type() != ir::Type::f32() ||
      operation_->result(0).type() != ir::Type::f32()) {
    error =
        "mini.scalar_relu requires one f32 operand, one f32 result, and no "
        "regions";
    return false;
  }
  return true;
}

}  // namespace mini::ops
