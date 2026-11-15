#pragma once

#include "mini/analysis/UseCountAnalysis.h"

#include <memory>
#include <unordered_map>

namespace mini::analysis {

// Caches function analyses. A pass manager invalidates entries after changes.
class AnalysisManager {
 public:
  const UseCountAnalysis& getUseCount(const ir::Operation& function);
  void invalidate(const ir::Operation& function);

 private:
  std::unordered_map<const ir::Operation*, std::unique_ptr<UseCountAnalysis>>
      useCounts_;
};

}  // namespace mini::analysis
