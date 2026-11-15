#pragma once

#include "mini/ops/StructuredOpInfo.h"

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::ops {

// A typed, non-owning view of a mini.add Operation.
class MiniAddOp {
 public:
  static constexpr std::string_view name = "mini.add";

  explicit MiniAddOp(const ir::Operation& operation);
  bool verify(std::string& error) const;
  StructuredOpInfo structuredInfo() const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
