#include "mini/ops/VectorOps.h"

#include <stdexcept>

#include "mini/ir/Operation.h"

namespace mini::ops {
namespace {

bool validShape(const ir::Operation& operation, std::size_t operandCount,
                std::size_t resultCount) {
  return operation.operands().size() == operandCount &&
         operation.resultCount() == resultCount && operation.regions().empty();
}

}  // namespace

VectorFromTensorOp::VectorFromTensorOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) throw std::invalid_argument("expected vector.from_tensor");
}

bool VectorFromTensorOp::verify(std::string& error) const {
  error.clear();
  if (!validShape(*operation_, inputs.size(), outputs.size()) ||
      operation_->operands()[0]->type() != ir::Type::tensor({4}) ||
      operation_->result(0).type() != ir::Type::vector({4})) {
    error = "vector.from_tensor requires tensor<4xf32> -> vector<4xf32>";
    return false;
  }
  return true;
}

VectorAddOp::VectorAddOp(const ir::Operation& operation) : operation_(&operation) {
  if (operation.name() != name) throw std::invalid_argument("expected vector.add");
}

bool VectorAddOp::verify(std::string& error) const {
  error.clear();
  if (!validShape(*operation_, inputs.size(), outputs.size()) ||
      operation_->operands()[0]->type() != ir::Type::vector({4}) ||
      operation_->operands()[1]->type() != ir::Type::vector({4}) ||
      operation_->result(0).type() != ir::Type::vector({4})) {
    error = "vector.add requires two vector<4xf32> operands and one matching result";
    return false;
  }
  return true;
}

VectorReluOp::VectorReluOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) throw std::invalid_argument("expected vector.relu");
}

bool VectorReluOp::verify(std::string& error) const {
  error.clear();
  if (!validShape(*operation_, inputs.size(), outputs.size()) ||
      operation_->operands()[0]->type() != ir::Type::vector({4}) ||
      operation_->result(0).type() != ir::Type::vector({4})) {
    error = "vector.relu requires vector<4xf32> operand and result";
    return false;
  }
  return true;
}

VectorToTensorOp::VectorToTensorOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name) throw std::invalid_argument("expected vector.to_tensor");
}

bool VectorToTensorOp::verify(std::string& error) const {
  error.clear();
  if (!validShape(*operation_, inputs.size(), outputs.size()) ||
      operation_->operands()[0]->type() != ir::Type::vector({4}) ||
      operation_->result(0).type() != ir::Type::tensor({4})) {
    error = "vector.to_tensor requires vector<4xf32> -> tensor<4xf32>";
    return false;
  }
  return true;
}

}  // namespace mini::ops
