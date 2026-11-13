#include "mini/ir/Value.h"

#include <stdexcept>
#include <utility>

namespace mini::ir {

Value::Value(Type type, Operation* definingOp, std::size_t resultIndex)
    : type_(std::move(type)),
      definingOp_(definingOp),
      resultIndex_(resultIndex) {}

void Value::addUse(Operation* owner, std::size_t operandIndex) {
  // TODO(lesson 2.1): register {owner, operandIndex} exactly once.
  throw std::logic_error("TODO: Value::addUse");
}

void Value::removeUse(Operation* owner, std::size_t operandIndex) {
  // TODO(lesson 2.1): remove the matching use; reject a missing use.
  throw std::logic_error("TODO: Value::removeUse");
}

}  // namespace mini::ir
