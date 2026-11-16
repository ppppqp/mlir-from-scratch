#include "mini/ops/TensorEmptyOp.h"

#include <stdexcept>

#include "mini/ir/Operation.h"

namespace mini::ops {

TensorEmptyOp::TensorEmptyOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected tensor.empty operation");
  }
}

bool TensorEmptyOp::verify(std::string& error) const {
  error.clear();
  if (!operation_->operands().empty() || operation_->resultCount() != 1 ||
      !operation_->regions().empty()) {
    error = "tensor.empty requires no operands, one result, and no regions";
    return false;
  }
  const ir::Type& type = operation_->result(0).type();
  if (type.kind() != ir::Type::Kind::Tensor || type.shape().size() != 1 ||
      type.shape()[0] <= 0) {
    error = "tensor.empty result must be a positive rank-1 tensor";
    return false;
  }
  return true;
}

}  // namespace mini::ops
