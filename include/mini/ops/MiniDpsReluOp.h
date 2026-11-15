#pragma once

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

// mini.dps_relu(A, init) -> result. init is the destination operand;
// its old elements are not read by the elementwise computation.
class MiniDpsReluOp {
 public:
  static constexpr std::string_view name = "mini.dps_relu";
  static constexpr unsigned outputOperandIndex = 1;

  explicit MiniDpsReluOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
