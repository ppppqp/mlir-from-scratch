#include "mini/ir/Operation.h"

#include <stdexcept>
#include <utility>

namespace mini::ir {

Operation::Operation(std::string name, std::vector<Value*> operands,
                     std::vector<Type> resultTypes)
    : name_(std::move(name)), operands_(std::move(operands)) {
  // TODO(lesson 2.1): create one stable Value per result type, then register
  // each operand use with its index. Reject null operands.
  (void)resultTypes;
  throw std::logic_error("TODO: Operation constructor");
}

Operation::~Operation() {
  // TODO(lesson 2.1): unregister operand uses. A result must have no remaining
  // users before its defining Operation is destroyed. Destructors must not
  // throw; an assertion is appropriate for this development-time invariant.
}

void Operation::setOperand(std::size_t index, Value* replacement) {
  // TODO(lesson 2.1): check the index and replacement, remove the old Use,
  // replace the pointer, then register the new Use.
  (void)index;
  (void)replacement;
  throw std::logic_error("TODO: Operation::setOperand");
}

}  // namespace mini::ir
