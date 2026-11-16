#pragma once

#include "mini/pass/Pass.h"

namespace mini::pass {

// Lesson 7.2: tile one-dimensional mini.generic operations in groups of four.
class TileGenericPass final : public Pass {
 public:
  std::string_view name() const override { return "tile-generic"; }
  bool run(ir::Operation& operation, std::string& error) override;
};

}  // namespace mini::pass
