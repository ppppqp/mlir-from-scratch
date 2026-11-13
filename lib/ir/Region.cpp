#include "mini/ir/Region.h"

#include <stdexcept>

#include "mini/ir/Block.h"

namespace mini::ir {

Region::~Region() = default;

Block& Region::addBlock() {
  // TODO(lesson 2.2): create and own one Block, then return it.
  // Reject adding a second Block in the initial single-block subset.
  throw std::logic_error("TODO: Region::addBlock");
}

}  // namespace mini::ir
