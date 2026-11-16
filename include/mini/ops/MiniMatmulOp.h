#pragma once

#include "mini/ops/StructuredOpInfo.h"
#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

class MiniMatmulOp {
 public:
  static constexpr std::string_view name = "mini.matmul";
  static constexpr auto inputs = std::array{PortSpec{"lhs", "tensor<MxKxf32>"},
                                            PortSpec{"rhs", "tensor<KxNxf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<MxNxf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};

  explicit MiniMatmulOp(const ir::Operation& operation);
  static constexpr bool hasSideEffects() { return false; }
  bool verify(std::string& error) const;
  StructuredOpInfo structuredInfo() const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
