#pragma once

#include "mini/pass/Pass.h"

namespace mini::pass {

// Lesson 7.3: vectorize add -> relu generic bodies on tensor<4xf32> tiles.
class VectorizeGenericPass final : public Pass {
 public:
  std::string_view name() const override { return "vectorize-generic"; }
  bool run(ir::Operation& operation, std::string& error) override;
};

}  // namespace mini::pass
