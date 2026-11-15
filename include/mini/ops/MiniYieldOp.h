#pragma once

#include <string>
#include <string_view>

namespace mini::ir { class Operation; }

namespace mini::ops {

class MiniYieldOp {
 public:
  static constexpr std::string_view name = "mini.yield";
  explicit MiniYieldOp(const ir::Operation& operation);
  bool verify(std::string& error) const;

 private:
  const ir::Operation* operation_;
};

}  // namespace mini::ops
