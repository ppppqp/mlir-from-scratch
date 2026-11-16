#pragma once

#include "mini/ops/OpSchema.h"

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

// One-dimensional subset. Bounds and step are int64 attributes; the body
// receives an index induction variable and a tensor loop-carried value.
class ScfForOp {
 public:
  static constexpr std::string_view name = "scf.for";
  static constexpr auto inputs = std::array{PortSpec{"init", "tensor<Nxf32>"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<Nxf32>"}};
  static constexpr auto attributes = std::array{
      AttributeSpec{"lower_bound", "i64"},
      AttributeSpec{"upper_bound", "i64"}, AttributeSpec{"step", "i64"}};
  static constexpr std::string_view regionSignature =
      "(iv: index, iter_arg: tensor<Nxf32>) -> tensor<Nxf32> via scf.yield";
  explicit ScfForOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

class ScfYieldOp {
 public:
  static constexpr std::string_view name = "scf.yield";
  static constexpr auto inputs = std::array{PortSpec{"value", "tensor<Nxf32>"}};
  static constexpr std::array<PortSpec, 0> outputs{};
  static constexpr std::array<AttributeSpec, 0> attributes{};
  explicit ScfYieldOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

// A padded slice of tile_size elements at an index offset. Out-of-bounds
// lanes are zero; this is a teaching subset, not MLIR extract_slice semantics.
class TensorExtractSliceOp {
 public:
  static constexpr std::string_view name = "tensor.extract_slice";
  static constexpr auto inputs = std::array{PortSpec{"source", "tensor<Nxf32>"},
                                            PortSpec{"offset", "index"}};
  static constexpr auto outputs =
      std::array{PortSpec{"result", "tensor<tile_size x f32>"}};
  static constexpr auto attributes =
      std::array{AttributeSpec{"tile_size", "i64"}};
  explicit TensorExtractSliceOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

// Insert only min(tile_size, destination_length - offset) valid lanes.
class TensorInsertSliceOp {
 public:
  static constexpr std::string_view name = "tensor.insert_slice";
  static constexpr auto inputs = std::array{
      PortSpec{"tile", "tensor<tile_size x f32>"},
      PortSpec{"destination", "tensor<Nxf32>"}, PortSpec{"offset", "index"}};
  static constexpr auto outputs = std::array{PortSpec{"result", "tensor<Nxf32>"}};
  static constexpr auto attributes =
      std::array{AttributeSpec{"tile_size", "i64"}};
  explicit TensorInsertSliceOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
