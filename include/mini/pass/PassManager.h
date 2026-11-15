#pragma once

#include "mini/pass/Pass.h"
#include "mini/analysis/AnalysisManager.h"

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace mini::pass {

// Run passes and nested managers in the order they were added.
class PassManager {
 public:
  explicit PassManager(std::string targetOpName);

  std::string_view targetOpName() const { return targetOpName_; }
  void addPass(std::unique_ptr<Pass> pass);
  PassManager& nest(std::string targetOpName);
  bool run(ir::Operation& operation, std::string& error);
  analysis::AnalysisManager& analyses() { return analyses_; }

 private:
  using Step =
      std::variant<std::unique_ptr<Pass>, std::unique_ptr<PassManager>>;

  std::string targetOpName_;
  std::vector<Step> steps_;
  analysis::AnalysisManager analyses_;
};

}  // namespace mini::pass
