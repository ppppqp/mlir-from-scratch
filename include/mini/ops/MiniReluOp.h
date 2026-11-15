#pragma once

#include "mini/ops/StructuredOpInfo.h"

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

class MiniReluOp {
 public:
  static constexpr std::string_view name = "mini.relu";

  explicit MiniReluOp(const ir::Operation& operation);
  bool verify(std::string& error) const;
  StructuredOpInfo structuredInfo() const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
