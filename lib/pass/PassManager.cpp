#include "mini/pass/PassManager.h"

#include <stdexcept>
#include <utility>

#include "mini/ir/Operation.h"

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
  passes_.push_back(std::move(pass));
}

bool PassManager::run(ir::Operation& operation, std::string& error) {
  error.clear();
  if (operation.name() != targetOpName_) {
    error = "pass manager expected " + targetOpName_ + ", got " +
            std::string(operation.name());
    return false;
  }
  for (auto const& pass : passes_) {
    if (!pass->run(operation, error)) {
      error = "pass " + std::string(pass->name()) +
              " failed: " + (error.empty() ? "no diagnostic" : error);
      return false;
    }
    error.clear();
  }
  return true;
}

}  // namespace mini::pass
