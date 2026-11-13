#include "mini/ir/ModuleOp.h"

#include <stdexcept>

#include "assert.h"
#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
namespace mini::ir {

ModuleOp::ModuleOp(Operation& operation) : operation_(&operation) {
  // TODO(lesson 2.2): reject an Operation whose name is not "builtin.module".
  if (operation.name() != "builtin.module") {
    throw std::invalid_argument("module op name is incorrect");
  }
}

Block& ModuleOp::body() const {
  // TODO(lesson 2.2): return the only Block in the only Region.
  auto regions = operation_->regions();
  if (regions.size() != 1 || regions[0]->blocks().size() != 1) {
    throw std::logic_error("expected one region with one block");
  }
  return *regions[0]->blocks()[0];
}
}  // namespace mini::ir
