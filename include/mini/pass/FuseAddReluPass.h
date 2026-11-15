#pragma once

#include "mini/pass/Pass.h"

namespace mini::pass {

// Lesson 6.5: fuse a single-use tensor add followed by relu.
class FuseAddReluPass final : public Pass {
 public:
  std::string_view name() const override { return "fuse-add-relu"; }
  bool run(ir::Operation& operation, std::string& error) override;
};

}  // namespace mini::pass
