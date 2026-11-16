#pragma once

#include "mini/ops/StructuredOpInfo.h"
#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

class MiniReluOp {
 public:
  static constexpr std::string_view name = "mini.relu";
  static constexpr auto inputs = std::array{PortSpec{"input", "tensor<Nxf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<Nxf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};

  explicit MiniReluOp(const ir::Operation& operation);
  static constexpr bool hasSideEffects() { return false; }
  bool verify(std::string& error) const;
  StructuredOpInfo structuredInfo() const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
