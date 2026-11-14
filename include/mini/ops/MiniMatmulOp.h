#pragma once

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

class MiniMatmulOp {
 public:
  static constexpr std::string_view name = "mini.matmul";

  explicit MiniMatmulOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
