#pragma once

#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

class MiniYieldOp {
 public:
  static constexpr std::string_view name = "mini.yield";
  static constexpr auto inputs = std::array{PortSpec{"value", "f32"}};
  static constexpr std::array<PortSpec, 0> outputs{};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  explicit MiniYieldOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
