#include "mini/ir/Block.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
namespace mini::ir {

Block::~Block() {
  // TODO(lesson 2.2): pop operations_ from the back until empty. Then the
  // argument Values will be destroyed by their owning vector.
  while (!operations_.empty()) {
    operations_.pop_back();
  }
}

Value& Block::addArgument(Type type) {
  // TODO(lesson 2.2): create a Value owned by this Block, with an argument
  // index equal to the current number of arguments. Keep its address stable.
  arguments_.push_back(std::make_unique<Value>(type, this, arguments_.size()));
  return *arguments_.back();
}

Operation& Block::append(std::unique_ptr<Operation> operation) {
  // TODO(lesson 2.2): reject null, take ownership, and return the appended op.
  if (operation == nullptr) {
    throw std::invalid_argument("operation can not be null");
  }

  operations_.push_back(std::move(operation));
  return *operations_.back();
}

Operation& Block::insertBefore(Operation& anchor,
                               std::unique_ptr<Operation> operation) {
  if (!operation) {
    throw std::invalid_argument("operation can not be null");
  }
  const auto it = std::find_if(
      operations_.begin(), operations_.end(),
      [&anchor](const std::unique_ptr<Operation>& owned) {
        return owned.get() == &anchor;
      });
  if (it == operations_.end()) {
    throw std::invalid_argument("anchor not owned by the block");
  }
  Operation* inserted = operation.get();
  operations_.insert(it, std::move(operation));
  return *inserted;
}

void Block::erase(Operation& operation) {
  // TODO(lesson 2.3): reject an operation not owned by this Block or one whose
  // results still have uses. Erase only when its destruction is safe.
  (void)operation;
  const auto it =
      std::find_if(operations_.begin(), operations_.end(),
                   [&operation](const std::unique_ptr<Operation>& op) {
                     return op.get() == &operation;
                   });
  if (it == operations_.end()) {
    throw std::logic_error("operation not owned by the block");
  }
  for (size_t i = 0; i < operation.resultCount(); ++i) {
    if (operation.result(i).uses().size()) {
      throw std::logic_error("operation results still have uses");
    }
  }

  operations_.erase(it);
}

}  // namespace mini::ir
