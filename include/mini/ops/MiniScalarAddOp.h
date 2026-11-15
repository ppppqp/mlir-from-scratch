#pragma once

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

class MiniScalarAddOp {
 public:
  static constexpr std::string_view name = "mini.scalar_add";
  explicit MiniScalarAddOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
