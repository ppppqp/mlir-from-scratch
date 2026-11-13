#include "mini/ir/FuncOp.h"

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
  throw std::logic_error("TODO: FuncOp::entryBlock");
}

}  // namespace mini::ir
