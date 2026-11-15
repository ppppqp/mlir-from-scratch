#include "mini/ops/MiniYieldOp.h"

#include <stdexcept>

#include "mini/ir/Operation.h"

namespace mini::ops {

MiniYieldOp::MiniYieldOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected mini.yield operation");
  }
}

bool MiniYieldOp::verify(std::string& error) const {
  error.clear();
  // TODO(lesson 6.4): require one f32 operand, zero results, no regions.
  if (operation_->operands().size() != 1 || operation_->resultCount() != 0 ||
      !operation_->regions().empty() ||
      operation_->operands()[0]->type() != ir::Type::f32()) {
    error = "mini.yield requires one f32 operand, no results, and no regions";
    return false;
  }
  return true;
}

}  // namespace mini::ops
