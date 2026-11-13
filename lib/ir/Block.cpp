#include "mini/ir/Block.h"

#include <stdexcept>

namespace mini::ir {

Block::~Block() {
  // TODO(lesson 2.2): pop operations_ from the back until empty. Then the
  // argument Values will be destroyed by their owning vector.
}

Value& Block::addArgument(Type type) {
  // TODO(lesson 2.2): create a Value owned by this Block, with an argument
  // index equal to the current number of arguments. Keep its address stable.
  (void)type;
  throw std::logic_error("TODO: Block::addArgument");
}

Operation& Block::append(std::unique_ptr<Operation> operation) {
  // TODO(lesson 2.2): reject null, take ownership, and return the appended op.
  (void)operation;
  throw std::logic_error("TODO: Block::append");
}

}  // namespace mini::ir
