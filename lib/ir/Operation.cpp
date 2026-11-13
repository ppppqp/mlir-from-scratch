#include "mini/ir/Operation.h"

#include <assert.h>

#include <stdexcept>
#include <utility>
#include <vector>

#include "mini/ir/Region.h"
namespace mini::ir {

Operation::Operation(std::string name, std::vector<Value*> operands,
                     std::vector<Type> resultTypes)
    : name_(std::move(name)), operands_(std::move(operands)) {
  // TODO(lesson 2.1): create one stable Value per result type, then register
  // each operand use with its index. Reject null operands.
  this->results_.reserve(resultTypes.size());

  // validate all operands first
  for (size_t i = 0; i < operands_.size(); ++i) {
    if (operands_[i] == nullptr)
      throw std::invalid_argument("operand must not be null");
  }

  for (size_t i = 0; i < operands_.size(); ++i) {
    operands_[i]->addUse(this, i);
  }

  for (size_t i = 0; i < resultTypes.size(); ++i) {
    auto val = std::make_unique<Value>(resultTypes[i], this, i);
    this->results_.push_back(std::move(val));
  }
}

Operation::~Operation() {
  // Nested operations may use values defined by this operation. Destroy them
  // before checking this operation's results and unregistering its operands.
  regions_.clear();
  // TODO(lesson 2.1): unregister operand uses. A result must have no remaining
  // users before its defining Operation is destroyed. Destructors must not
  // throw; an assertion is appropriate for this development-time invariant.
  for (size_t i = 0; i < results_.size(); ++i) {
    assert(results_[i]->uses_.size() == 0);
  }
  for (size_t i = 0; i < operands_.size(); ++i) {
    operands_[i]->removeUse(this, i);
  }
}

Region& Operation::addRegion() {
  // TODO(lesson 2.2): create and own a Region, then return it.
}

void Operation::setOperand(std::size_t index, Value* replacement) {
  // TODO(lesson 2.1): check the index and replacement, remove the old Use,
  // replace the pointer, then register the new Use.
  if (index >= this->operands_.size())
    throw std::out_of_range("Invalid operand index");
  if (replacement == nullptr) {
    throw std::invalid_argument("Replacement cannot be null");
  }
  Value* original_operand = this->operands_[index];
  original_operand->removeUse(this, index);

  this->operands_[index] = replacement;
  replacement->addUse(this, index);
}

}  // namespace mini::ir
