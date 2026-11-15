#pragma once

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

// mini.dps_add(A, B, init) -> result. init is the destination operand;
// its old elements are not read by the elementwise computation.
class MiniDpsAddOp {
 public:
  static constexpr std::string_view name = "mini.dps_add";
  static constexpr unsigned outputOperandIndex = 2;

  explicit MiniDpsAddOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
