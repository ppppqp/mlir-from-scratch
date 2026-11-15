#include "mini/pass/PassManager.h"

#include <stdexcept>
#include <utility>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
namespace mini::pass {

PassManager::PassManager(std::string targetOpName)
    : targetOpName_(std::move(targetOpName)) {
  if (targetOpName_.empty()) {
    throw std::invalid_argument("pass manager target operation name is empty");
  }
}

void PassManager::addPass(std::unique_ptr<Pass> pass) {
  if (!pass) {
    throw std::invalid_argument("pass must not be null");
  }
  steps_.emplace_back(std::move(pass));
}

PassManager& PassManager::nest(std::string targetOpName) {
  auto nested = std::make_unique<PassManager>(std::move(targetOpName));
  PassManager& result = *nested;
  steps_.emplace_back(std::move(nested));
  return result;
}

bool PassManager::run(ir::Operation& operation, std::string& error) {
  error.clear();
  if (operation.name() != targetOpName_) {
    error = "pass manager expected " + targetOpName_ + ", got " +
            std::string(operation.name());
    return false;
  }
  for (auto& step : steps_) {
    if (auto* pass = std::get_if<std::unique_ptr<Pass>>(&step)) {
      if (!(*pass)->run(operation, error)) {
        // A failing pass may already have changed the IR.
        analyses_.invalidate(operation);
        error = "pass " + std::string((*pass)->name()) +
                " failed: " + (error.empty() ? "no diagnostic" : error);
        return false;
      }
      if (!(*pass)->preservesUseCountAnalysis()) {
        analyses_.invalidate(operation);
      }
      error.clear();
      continue;
    }

    PassManager& nested = *std::get<std::unique_ptr<PassManager>>(step);
    for (const auto& region : operation.regions()) {
      for (const auto& block : region->blocks()) {
        for (const auto& child : block->operations()) {
          if (child->name() == nested.targetOpName() &&
              !nested.run(*child, error)) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

}  // namespace mini::pass
