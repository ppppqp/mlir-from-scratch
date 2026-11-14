#pragma once

#include <memory>
#include <string_view>

namespace mini::ir {

class Operation;

// Parse only the MLIR-style subset in docs/lesson03/01-text-syntax.md.
// The returned Operation owns the entire builtin.module tree.
std::unique_ptr<Operation> parseModule(std::string_view source);

}  // namespace mini::ir
