#pragma once

#include "mini/pass/Pass.h"

namespace mini::pass {

// Lesson 6.3: eliminate duplicate pure computations in one function block.
class CsePass final : public Pass {
 public:
  std::string_view name() const override { return "cse"; }
  bool run(ir::Operation& operation, std::string& error) override;
};

}  // namespace mini::pass
