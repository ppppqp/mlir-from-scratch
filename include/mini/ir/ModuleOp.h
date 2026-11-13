#pragma once

namespace mini::ir {

class Block;
class Operation;

// A typed, non-owning view of an Operation named "builtin.module".
// The caller owns the root Operation, e.g. with std::unique_ptr<Operation>.
class ModuleOp {
 public:
  explicit ModuleOp(Operation& operation);

  Operation& operation() const { return *operation_; }
  Block& body() const;

 private:
  Operation* operation_;
};

}  // namespace mini::ir
