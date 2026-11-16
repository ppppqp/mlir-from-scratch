#pragma once

#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

class MiniScalarAddOp {
 public:
  static constexpr std::string_view name = "mini.scalar_add";
  static constexpr auto inputs = std::array{PortSpec{"lhs", "f32"},
                                            PortSpec{"rhs", "f32"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "f32"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  explicit MiniScalarAddOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
