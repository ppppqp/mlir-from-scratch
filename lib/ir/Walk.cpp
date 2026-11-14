#include "mini/ir/Walk.h"

#include <stdexcept>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"

namespace mini::ir {

void walk(Operation& root, const std::function<void(Operation&)>& callback) {
  // TODO(lesson 2.3): call callback(root), then recursively walk all ops in
  // every Block of every Region owned by root, in storage order.
  callback(root);
  for (const auto& region : root.regions()) {
    for (const auto& block : region->blocks()) {
      for (const auto& op : block->operations()) {
        walk(*op, callback);
      }
    }
  }
}

}  // namespace mini::ir
