#include "mini/ops/MiniReluOp.h"

#include <stdexcept>

#include "mini/ir/Operation.h"

namespace mini::ops {

MiniReluOp::MiniReluOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected mini.relu operation");
  }
}

bool MiniReluOp::verify(std::string& error) const {
  error.clear();
  const auto& operation = *operation_;
  if (operation.operands().size() != 1) {
    error = "mini.relu must have one operand";
    return false;
  }
  if (operation.resultCount() != 1 || !operation.regions().empty()) {
    error = "mini.relu must have one result and no regions";
    return false;
  }
  const ir::Type& input = operation.operands()[0]->type();
  if (input.kind() != ir::Type::Kind::Tensor || input.shape().size() != 1) {
    error = "mini.relu operand must be a rank-1 tensor";
    return false;
  }
  if (input.shape()[0] <= 0 || input.shape()[0] % 4 != 0) {
    error = "mini.relu length must be positive and divisible by 4";
    return false;
  }
  if (operation.result(0).type() != input) {
    error = "mini.relu result type must match its operand";
    return false;
  }
  return true;
}

StructuredOpInfo MiniReluOp::structuredInfo() const {
  std::string error;
  if (!verify(error)) {
    throw std::invalid_argument(error);
  }
  const auto length = operation_->operands()[0]->type().shape()[0];
  return StructuredOpInfo{
      {length}, {IndexingMap{{0}}, IndexingMap{{0}}}, {IteratorKind::Parallel}};
}

}  // namespace mini::ops
