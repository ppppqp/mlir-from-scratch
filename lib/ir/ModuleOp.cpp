#include "mini/ir/ModuleOp.h"

#include <stdexcept>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"

namespace mini::ir {

ModuleOp::ModuleOp(Operation& operation) : operation_(&operation) {
  // TODO(lesson 2.2): reject an Operation whose name is not "builtin.module".
}

Block& ModuleOp::body() const {
  // TODO(lesson 2.2): return the only Block in the only Region.
  throw std::logic_error("TODO: ModuleOp::body");
}

}  // namespace mini::ir
