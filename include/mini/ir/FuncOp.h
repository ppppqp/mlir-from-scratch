#pragma once

#include <string>

namespace mini::ir {

class Block;
class Operation;

// A typed, non-owning view of an Operation named "func.func".
// Function name/signature attributes are added with the textual IR in lesson 3.
class FuncOp {
 public:
  explicit FuncOp(Operation& operation);

  Operation& operation() const { return *operation_; }
  Block& entryBlock() const;

  // Verify the function body against its function_type attribute.
  // Return true on success and clear error; otherwise explain the first error.
  bool verifyRegions(std::string& error) const;

 private:
  Operation* operation_;
};

}  // namespace mini::ir
