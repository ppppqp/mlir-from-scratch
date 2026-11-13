#include "mini/ir/FuncOp.h"

#include <assert.h>

#include <stdexcept>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
namespace mini::ir {

FuncOp::FuncOp(Operation& operation) : operation_(&operation) {
  // TODO(lesson 2.2): reject an Operation whose name is not "func.func".
}

Block& FuncOp::entryBlock() const {
  // TODO(lesson 2.2): return the only Block in the only Region.
  auto regions = operation_->regions();
  if (regions.size() != 1 || regions[0]->blocks().size() != 1) {
    throw std::logic_error("expected one region with one block");
  }
  return *regions[0]->blocks()[0];
}

}  // namespace mini::ir
