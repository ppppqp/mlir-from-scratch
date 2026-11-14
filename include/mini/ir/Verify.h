#pragma once

#include <string>

namespace mini::ir {

class Operation;

// Dispatch to the typed verifier for a supported computation operation.
// Return true on success and clear error; otherwise return false with the
// first diagnostic. Module/function structural verification is separate.
bool verifyOperation(const Operation& operation, std::string& error);

// Verify a complete builtin.module, including its functions and nested
// computation operations. The current FuncOp view takes Operation&, but this
// function only reads the IR.
bool verifyModule(Operation& module, std::string& error);

}  // namespace mini::ir
