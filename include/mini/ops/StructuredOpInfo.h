#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mini::ops {

enum class IteratorKind { Parallel, Reduction };

// Each entry selects one iteration dimension. For example {0, 2} maps
// (m, n, k) to (m, k).
struct IndexingMap {
  std::vector<std::size_t> dimensions;
  bool operator==(const IndexingMap&) const = default;
};

struct StructuredOpInfo {
  std::vector<std::int64_t> iterationShape;
  // Input maps in operand order, followed by one output map.
  std::vector<IndexingMap> indexingMaps;
  std::vector<IteratorKind> iteratorKinds;
};

}  // namespace mini::ops
