#pragma once

#include <string>

namespace mini::ir {

class Operation;

// Print a parseable, deterministic form. SSA names may be canonicalized.
std::string printModule(const Operation& module);

}  // namespace mini::ir
