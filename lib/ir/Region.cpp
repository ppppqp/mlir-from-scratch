#include "mini/ir/Region.h"

#include <stdexcept>

#include "mini/ir/Block.h"

namespace mini::ir {

Region::~Region() = default;
Region::Region() = default;

Block& Region::addBlock() {
  // TODO(lesson 2.2): create and own one Block, then return it.
  // Reject adding a second Block in the initial single-block subset.
  if (blocks_.size() > 0) {
    throw std::overflow_error("only support single-block");
  }
  blocks_.push_back(std::make_unique<Block>());
  return *blocks_.back();
}

}  // namespace mini::ir
