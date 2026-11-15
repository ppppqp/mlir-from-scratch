#pragma once

#include <string>
#include <string_view>

namespace mini::ir {
class Operation;
}

namespace mini::pass {

class Pass {
 public:
  virtual ~Pass() = default;

  virtual std::string_view name() const = 0;
  // Override only when this pass cannot change operand use counts.
  virtual bool preservesUseCountAnalysis() const { return false; }
  // Return false and set error when the pass cannot complete.
  virtual bool run(ir::Operation& operation, std::string& error) = 0;
};

}  // namespace mini::pass
