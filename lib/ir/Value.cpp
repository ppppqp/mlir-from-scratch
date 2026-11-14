#include "mini/ir/Value.h"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>

#include "mini/ir/Operation.h"
namespace mini::ir {

Value::Value(Type type, Operation* definingOp, std::size_t resultIndex)
    : type_(std::move(type)),
      definingOp_(definingOp),
      resultIndex_(resultIndex),
      definingBlock_(nullptr),
      argumentIndex_(0) {}

Value::Value(Type type, Block* definingBlock, std::size_t argumentIndex)
    : type_(std::move(type)),
      definingOp_(nullptr),
      resultIndex_(0),
      definingBlock_(definingBlock),
      argumentIndex_(argumentIndex) {
  // TODO(lesson 2.2): reject a null definingBlock.
  if (definingBlock == nullptr) {
    throw std::invalid_argument("defining block can not be null");
  }
}

void Value::addUse(Operation* owner, std::size_t operandIndex) {
  // TODO(lesson 2.1): register {owner, operandIndex} exactly once.
  this->uses_.push_back({owner, operandIndex});
}

void Value::removeUse(Operation* owner, std::size_t operandIndex) noexcept {
  const auto it = std::find_if(
      uses_.begin(), uses_.end(), [owner, operandIndex](const Use& use) {
        return use.owner == owner && use.operandIndex == operandIndex;
      });
  if (it == uses_.end()) {
    std::terminate();
  }
  uses_.erase(it);
}

void Value::replaceAllUsesWith(Value& replacement) {
  // TODO(lesson 2.3): require matching types; replacing with *this is a no-op.
  // Copy uses_ before calling Operation::setOperand, because that call mutates
  // uses_. Afterward this Value should have no uses.

  if (this == &replacement) {
    return;
  }
  if (type_ != replacement.type()) {
    throw std::invalid_argument("type does not match");
  }
  std::vector<Use> uses_copy(uses_);
  for (auto const& use : uses_copy) {
    use.owner->setOperand(use.operandIndex, &replacement);
  }
}

}  // namespace mini::ir
