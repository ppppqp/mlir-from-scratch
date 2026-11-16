#pragma once

#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

class MiniScalarReluOp {
 public:
  static constexpr std::string_view name = "mini.scalar_relu";
  static constexpr auto inputs = std::array{PortSpec{"input", "f32"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "f32"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  explicit MiniScalarReluOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
