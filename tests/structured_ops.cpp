#include "mini/ir/Operation.h"
#include "mini/ops/MiniAddOp.h"
#include "mini/ops/MiniMatmulOp.h"
#include "mini/ops/MiniReluOp.h"

#include <cassert>
#include <vector>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;
  using mini::ops::IndexingMap;
  using mini::ops::IteratorKind;

  const Type vector4 = Type::tensor({4});
  Value a(vector4);
  Value b(vector4);
  Operation add("mini.add", {&a, &b}, {vector4});
  const auto addInfo = mini::ops::MiniAddOp(add).structuredInfo();
  assert(addInfo.iterationShape == std::vector<std::int64_t>{4});
  assert((addInfo.indexingMaps ==
          std::vector<IndexingMap>{{{0}}, {{0}}, {{0}}}));
  assert(addInfo.iteratorKinds ==
         std::vector<IteratorKind>{IteratorKind::Parallel});

  Operation relu("mini.relu", {&a}, {vector4});
  const auto reluInfo = mini::ops::MiniReluOp(relu).structuredInfo();
  assert(reluInfo.iterationShape == std::vector<std::int64_t>{4});
  assert((reluInfo.indexingMaps == std::vector<IndexingMap>{{{0}}, {{0}}}));
  assert(reluInfo.iteratorKinds ==
         std::vector<IteratorKind>{IteratorKind::Parallel});

  const Type leftType = Type::tensor({16, 32});
  const Type rightType = Type::tensor({32, 16});
  const Type resultType = Type::tensor({16, 16});
  Value left(leftType);
  Value right(rightType);
  Operation matmul("mini.matmul", {&left, &right}, {resultType});
  const auto matmulInfo = mini::ops::MiniMatmulOp(matmul).structuredInfo();
  assert((matmulInfo.iterationShape == std::vector<std::int64_t>{16, 16, 32}));
  assert((matmulInfo.indexingMaps ==
          std::vector<IndexingMap>{{{0, 2}}, {{2, 1}}, {{0, 1}}}));
  assert((matmulInfo.iteratorKinds ==
          std::vector<IteratorKind>{IteratorKind::Parallel,
                                    IteratorKind::Parallel,
                                    IteratorKind::Reduction}));
}
