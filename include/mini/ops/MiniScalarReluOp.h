#pragma once

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

class MiniScalarReluOp {
 public:
  static constexpr std::string_view name = "mini.scalar_relu";
  explicit MiniScalarReluOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
