#pragma once

#include "mini/ir/Type.h"

#include <string>
#include <variant>
#include <vector>

namespace mini::ir {

// A func.func stores its signature as an attribute. The operation itself has
// no SSA results; these results describe values returned by a function call.
struct FunctionType {
  std::vector<Type> inputs;
  std::vector<Type> results;

  bool operator==(const FunctionType&) const = default;
};

using Attribute = std::variant<std::string, FunctionType>;

}  // namespace mini::ir
