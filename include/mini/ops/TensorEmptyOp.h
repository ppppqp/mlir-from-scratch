#pragma once

#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

// Creates an uninitialized rank-1 tensor. Every element must be overwritten
// before it is observed; the verifier checks only the local operation shape.
class TensorEmptyOp {
 public:
  static constexpr std::string_view name = "tensor.empty";
  static constexpr std::array<PortSpec, 0> inputs{};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<Nxf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  explicit TensorEmptyOp(const ir::Operation& operation);
  static constexpr bool hasSideEffects() { return false; }
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
