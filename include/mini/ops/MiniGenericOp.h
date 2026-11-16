#pragma once

#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

// Two-input elementwise generic op. Its region operates on f32 scalars.
class MiniGenericOp {
 public:
  static constexpr std::string_view name = "mini.generic";
  static constexpr auto inputs = std::array{PortSpec{"lhs", "tensor<Nxf32>"},
                                            PortSpec{"rhs", "tensor<Nxf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<Nxf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  static constexpr std::string_view regionSignature =
      "(lhs: f32, rhs: f32) -> f32 via mini.yield";
  explicit MiniGenericOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
