#include "mini/ops/MiniAddOp.h"

#include "mini/ir/Operation.h"

#include <stdexcept>

namespace mini::ops {

MiniAddOp::MiniAddOp(const ir::Operation& operation) : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected mini.add operation");
  }
}

bool MiniAddOp::verify(std::string& error) const {
  error.clear();
  const auto& operation = *operation_;
  if (operation.operands().size() != 2) {
    error = "mini.add must have two operands";
    return false;
  }
  if (operation.resultCount() != 1 || !operation.regions().empty()) {
    error = "mini.add must have one result and no regions";
    return false;
  }
  const ir::Type& lhs = operation.operands()[0]->type();
  const ir::Type& rhs = operation.operands()[1]->type();
  if (lhs != rhs) {
    error = "mini.add operand types must match";
    return false;
  }
  if (lhs.kind() != ir::Type::Kind::Tensor || lhs.shape().size() != 1) {
    error = "mini.add operands must be rank-1 tensors";
    return false;
  }
  if (lhs.shape()[0] <= 0 || lhs.shape()[0] % 4 != 0) {
    error = "mini.add length must be positive and divisible by 4";
    return false;
  }
  if (operation.result(0).type() != lhs) {
    error = "mini.add result type must match its operands";
    return false;
  }
  return true;
}

}  // namespace mini::ops
