#pragma once

#include "mini/ops/StructuredOpInfo.h"
#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

// A typed, non-owning view of a mini.add Operation.
class MiniAddOp {
 public:
  static constexpr std::string_view name = "mini.add";
  static constexpr auto inputs = std::array{PortSpec{"lhs", "tensor<Nxf32>"},
                                            PortSpec{"rhs", "tensor<Nxf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<Nxf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};

  explicit MiniAddOp(const ir::Operation& operation);
  static constexpr bool hasSideEffects() { return false; }
  bool verify(std::string& error) const;
  StructuredOpInfo structuredInfo() const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
