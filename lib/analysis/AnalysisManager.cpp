#include "mini/analysis/AnalysisManager.h"

#include "mini/ir/Operation.h"

namespace mini::analysis {

const UseCountAnalysis& AnalysisManager::getUseCount(
    const ir::Operation& function) {
  auto& entry = useCounts_[&function];
  if (!entry) {
    entry = std::make_unique<UseCountAnalysis>(function);
  }
  return *entry;
}

void AnalysisManager::invalidate(const ir::Operation& function) {
  auto it = useCounts_.find(&function);
  if (it != useCounts_.end()) {
    useCounts_.erase(&function);
  }
}

}  // namespace mini::analysis
