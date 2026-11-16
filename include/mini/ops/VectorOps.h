#pragma once

#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

class VectorFromTensorOp {
 public:
  static constexpr std::string_view name = "vector.from_tensor";
  static constexpr auto inputs = std::array{PortSpec{"input", "tensor<4xf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "vector<4xf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  static constexpr bool hasSideEffects() { return false; }
  explicit VectorFromTensorOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

class VectorAddOp {
 public:
  static constexpr std::string_view name = "vector.add";
  static constexpr auto inputs = std::array{PortSpec{"lhs", "vector<4xf32>"},
                                            PortSpec{"rhs", "vector<4xf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "vector<4xf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  static constexpr bool hasSideEffects() { return false; }
  explicit VectorAddOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

class VectorReluOp {
 public:
  static constexpr std::string_view name = "vector.relu";
  static constexpr auto inputs = std::array{PortSpec{"input", "vector<4xf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "vector<4xf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  static constexpr bool hasSideEffects() { return false; }
  explicit VectorReluOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

class VectorToTensorOp {
 public:
  static constexpr std::string_view name = "vector.to_tensor";
  static constexpr auto inputs = std::array{PortSpec{"input", "vector<4xf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<4xf32>"}};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  static constexpr bool hasSideEffects() { return false; }
  explicit VectorToTensorOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
