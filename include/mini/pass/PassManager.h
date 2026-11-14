#pragma once

#include "mini/pass/Pass.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace mini::pass {

// First stage: run an ordered sequence of passes on one Operation.
// Nested managers are added in the next exercise.
class PassManager {
 public:
  explicit PassManager(std::string targetOpName);

  std::string_view targetOpName() const { return targetOpName_; }
  void addPass(std::unique_ptr<Pass> pass);
  bool run(ir::Operation& operation, std::string& error);

 private:
  std::string targetOpName_;
  std::vector<std::unique_ptr<Pass>> passes_;
};

}  // namespace mini::pass
