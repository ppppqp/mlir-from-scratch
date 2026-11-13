#include "mini/ir/Block.h"

#include <stdexcept>

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

}  // namespace mini::ir
