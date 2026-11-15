#pragma once

#include "mini/ops/StructuredOpInfo.h"

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
  static constexpr bool hasSideEffects() { return false; }
  bool verify(std::string& error) const;
  StructuredOpInfo structuredInfo() const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
