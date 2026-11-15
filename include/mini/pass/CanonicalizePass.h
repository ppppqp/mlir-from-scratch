#pragma once

#include "mini/pass/Pass.h"

namespace mini::pass {

// Lesson 6.1: function-level canonicalization.
class CanonicalizePass final : public Pass {
 public:
  std::string_view name() const override { return "canonicalize"; }
  bool run(ir::Operation& operation, std::string& error) override;
};

}  // namespace mini::pass
