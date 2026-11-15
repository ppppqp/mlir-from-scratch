#pragma once

#include <cstddef>
#include <optional>
#include <unordered_map>

namespace mini::ir {
class Operation;
class Value;
}

namespace mini::analysis {

// A snapshot of operand-use counts for one function's entry block.
// Build a new analysis after changing the function IR.
class UseCountAnalysis {
 public:
  explicit UseCountAnalysis(const ir::Operation& function);

  // nullopt means the Value is not defined in this function; zero means it is
  // defined here but currently unused.
  std::optional<std::size_t> useCount(const ir::Value& value) const;

 private:
  std::unordered_map<const ir::Value*, std::size_t> counts_;
};

}  // namespace mini::analysis
