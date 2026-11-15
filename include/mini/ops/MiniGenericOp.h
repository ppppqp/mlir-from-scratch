#pragma once

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

// Two-input elementwise generic op. Its region operates on f32 scalars.
class MiniGenericOp {
 public:
  static constexpr std::string_view name = "mini.generic";
  explicit MiniGenericOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
