#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace mini::ir {

// Lesson 2.1: only f32 scalars and statically shaped f32 tensors.
class Type {
 public:
  enum class Kind { F32, Tensor };

  static Type f32();
  static Type tensor(std::vector<std::int64_t> shape);

  Kind kind() const { return kind_; }
  std::span<const std::int64_t> shape() const { return shape_; }
  bool operator==(const Type&) const = default;

 private:
  Type(Kind kind, std::vector<std::int64_t> shape);

  Kind kind_;
  std::vector<std::int64_t> shape_;
};

}  // namespace mini::ir
