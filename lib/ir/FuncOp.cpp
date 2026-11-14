#include "mini/ir/FuncOp.h"

#include <stdexcept>
#include <variant>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
namespace mini::ir {

FuncOp::FuncOp(Operation& operation) : operation_(&operation) {
  // TODO(lesson 2.2): reject an Operation whose name is not "func.func".
  if (operation.name() != "func.func") {
    throw std::invalid_argument("func op name is incorrect");
  }
}

Block& FuncOp::entryBlock() const {
  // TODO(lesson 2.2): return the only Block in the only Region.
  auto regions = operation_->regions();
  if (regions.size() != 1 || regions[0]->blocks().size() != 1) {
    throw std::logic_error("expected one region with one block");
  }
  return *regions[0]->blocks()[0];
}

bool FuncOp::verifyRegions(std::string& error) const {
  error.clear();
  const auto* function_type =
      std::get_if<FunctionType>(operation().findAttribute("function_type"));
  if (function_type == nullptr) {
    error = "func.func requires a function_type attribute";
    return false;
  }
  if (function_type->results.size() != 1) {
    error = "func.func must declare exactly one result type";
    return false;
  }
  if (operation().regions().size() != 1 ||
      operation().regions()[0]->blocks().size() != 1) {
    error = "func.func must have one region with one block";
    return false;
  }
  const Block& block = *operation().regions()[0]->blocks()[0];
  const auto arguments = block.arguments();
  if (arguments.size() != function_type->inputs.size()) {
    error = "entry block argument count does not match function_type inputs";
    return false;
  }
  for (std::size_t i = 0; i < arguments.size(); ++i) {
    if (arguments[i]->type() != function_type->inputs[i]) {
      error = "entry block argument type does not match function_type input";
      return false;
    }
  }
  if (block.operations().empty()) {
    error = "func.func body must end with func.return";
    return false;
  }
  const Operation& last_op = *block.operations().back();
  if (last_op.name() != "func.return" || last_op.resultCount() != 0 ||
      last_op.operands().size() != 1 ||
      last_op.operands()[0]->type() != function_type->results[0]) {
    error = "func.return does not match the function result type";
    return false;
  }

  return true;
}

}  // namespace mini::ir
