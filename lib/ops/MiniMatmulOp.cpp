#include "mini/ops/MiniMatmulOp.h"

#include "mini/ir/Operation.h"

#include <stdexcept>

namespace mini::ops {

MiniMatmulOp::MiniMatmulOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) {
    throw std::invalid_argument("expected mini.matmul operation");
  }
}

bool MiniMatmulOp::verify(std::string& error) const {
  error.clear();
  const auto& operation = *operation_;
  if (operation.operands().size() != 2) {
    error = "mini.matmul must have two operands";
    return false;
  }
  if (operation.resultCount() != 1 || !operation.regions().empty()) {
    error = "mini.matmul must have one result and no regions";
    return false;
  }
  const ir::Type& lhs = operation.operands()[0]->type();
  const ir::Type& rhs = operation.operands()[1]->type();
  const ir::Type& result = operation.result(0).type();
  const auto isMatrix = [](const ir::Type& type) {
    return type.kind() == ir::Type::Kind::Tensor &&
           type.shape().size() == 2;
  };
  if (!isMatrix(lhs) || !isMatrix(rhs) || !isMatrix(result)) {
    error = "mini.matmul operands and result must be rank-2 tensors";
    return false;
  }
  const auto a = lhs.shape();
  const auto b = rhs.shape();
  const auto c = result.shape();
  if (a[0] <= 0 || a[1] <= 0 || b[0] <= 0 || b[1] <= 0 ||
      c[0] <= 0 || c[1] <= 0 || a[0] % 16 != 0 || a[1] % 16 != 0 ||
      b[1] % 16 != 0) {
    error = "mini.matmul dimensions must be positive multiples of 16";
    return false;
  }
  if (a[1] != b[0] || c[0] != a[0] || c[1] != b[1]) {
    error = "mini.matmul inner or result dimensions do not match";
    return false;
  }
  return true;
}

}  // namespace mini::ops
