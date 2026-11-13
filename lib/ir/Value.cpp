#include "mini/ir/Value.h"

#include <algorithm>
#include <exception>
#include <utility>

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

}  // namespace mini::ir
