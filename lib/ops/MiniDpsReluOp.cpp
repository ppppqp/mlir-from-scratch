#include "mini/ops/MiniDpsReluOp.h"

#include <stdexcept>

#include "mini/ir/Operation.h"

namespace mini::ops {

MiniDpsReluOp::MiniDpsReluOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected mini.dps_relu operation");
  }
}

bool MiniDpsReluOp::verify(std::string& error) const {
  error.clear();
  if (operation_->operands().size() != 2) {
    error = "mini.dps_relu requires input and outs operands";
    return false;
  }
  if (operation_->resultCount() != 1 || !operation_->regions().empty()) {
    error = "mini.dps_relu requires one result and no regions";
    return false;
  }
  const ir::Type& input = operation_->operands()[0]->type();
  if (input.kind() != ir::Type::Kind::Tensor || input.shape().size() != 1 ||
      input.shape()[0] <= 0 || input.shape()[0] % 4 != 0) {
    error = "mini.dps_relu requires a rank-1 tensor with length divisible by 4";
    return false;
  }
  if (operation_->operands()[outputOperandIndex]->type() != input ||
      operation_->result(0).type() != input) {
    error = "mini.dps_relu input, outs, and result must have the same type";
    return false;
  }
  return true;
}

}  // namespace mini::ops
