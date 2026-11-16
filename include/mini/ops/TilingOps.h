#pragma once

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

// One-dimensional subset. Bounds and step are int64 attributes; the body
// receives an index induction variable and a tensor loop-carried value.
class ScfForOp {
 public:
  static constexpr std::string_view name = "scf.for";
  explicit ScfForOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

class ScfYieldOp {
 public:
  static constexpr std::string_view name = "scf.yield";
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
  explicit TensorExtractSliceOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

// Insert only min(tile_size, destination_length - offset) valid lanes.
class TensorInsertSliceOp {
 public:
  static constexpr std::string_view name = "tensor.insert_slice";
  explicit TensorInsertSliceOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
