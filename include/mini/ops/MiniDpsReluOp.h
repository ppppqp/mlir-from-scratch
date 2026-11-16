#pragma once

#include "mini/ops/OpSchema.h"

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
  static constexpr auto inputs = std::array{PortSpec{"input", "tensor<Nxf32>"},
                                            PortSpec{"init", "tensor<Nxf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<Nxf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  static constexpr unsigned outputOperandIndex = 1;

  explicit MiniDpsReluOp(const ir::Operation& operation);
  static constexpr bool hasSideEffects() { return true; }
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
