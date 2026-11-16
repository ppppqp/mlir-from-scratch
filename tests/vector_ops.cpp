#include "mini/ir/Block.h"
#include "mini/interpreter/Interpreter.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/TilingOps.h"
#include "mini/ops/VectorOps.h"
#include "mini/pass/PassManager.h"
#include "mini/pass/TileGenericPass.h"
#include "mini/pass/VectorizeGenericPass.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;
  static_assert(mini::ops::VectorAddOp::inputs.size() == 2);
  static_assert(mini::ops::ScfForOp::attributes.size() == 3);
  const Type tile = Type::tensor({4});
  const Type vector = Type::vector({4});
  Value a(tile);
  Value b(tile);
  std::string error;

  Operation va("vector.from_tensor", {&a}, {vector});
  Operation vb("vector.from_tensor", {&b}, {vector});
  Operation sum("vector.add", {&va.result(0), &vb.result(0)}, {vector});
  Operation relu("vector.relu", {&sum.result(0)}, {vector});
  Operation back("vector.to_tensor", {&relu.result(0)}, {tile});
  assert(mini::ir::verifyOperation(va, error));
  assert(mini::ir::verifyOperation(vb, error));
  assert(mini::ir::verifyOperation(sum, error));
  assert(mini::ir::verifyOperation(relu, error));
  assert(mini::ir::verifyOperation(back, error));
  Operation wrongWidth("vector.add", {&va.result(0), &vb.result(0)},
                       {Type::vector({8})});
  assert(!mini::ir::verifyOperation(wrongWidth, error));

  const Type full = Type::tensor({18});
  Operation function("func.func", {}, {});
  auto& block = function.addRegion().addBlock();
  Value& lhs = block.addArgument(full);
  Value& rhs = block.addArgument(full);
  Operation& generic = block.append(std::make_unique<Operation>(
      "mini.generic", std::vector<Value*>{&lhs, &rhs},
      std::vector<Type>{full}));
  auto& body = generic.addRegion().addBlock();
  Value& scalarLhs = body.addArgument(Type::f32());
  Value& scalarRhs = body.addArgument(Type::f32());
  Operation& scalarSum = body.append(std::make_unique<Operation>(
      "mini.scalar_add", std::vector<Value*>{&scalarLhs, &scalarRhs},
      std::vector<Type>{Type::f32()}));
  Operation& scalarRelu = body.append(std::make_unique<Operation>(
      "mini.scalar_relu", std::vector<Value*>{&scalarSum.result(0)},
      std::vector<Type>{Type::f32()}));
  body.append(std::make_unique<Operation>(
      "mini.yield", std::vector<Value*>{&scalarRelu.result(0)},
      std::vector<Type>{}));
  block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&generic.result(0)},
      std::vector<Type>{}));

  using mini::interpreter::TensorData;
  std::vector<TensorData> inputs{
      {full, std::vector<float>(18)}, {full, std::vector<float>(18)}};
  for (std::size_t i = 0; i < 18; ++i) {
    inputs[0].elements[i] = static_cast<float>(i) - 12.0f;
    inputs[1].elements[i] = 3.0f - static_cast<float>(i % 5);
  }
  const TensorData before = mini::interpreter::runFunction(function, inputs);

  mini::pass::PassManager manager("func.func");
  manager.addPass(std::make_unique<mini::pass::TileGenericPass>());
  manager.addPass(std::make_unique<mini::pass::VectorizeGenericPass>());
  assert(manager.run(function, error));
  const Operation& loop = *block.operations()[1];
  assert(loop.name() == "scf.for");
  assert(mini::ir::verifyOperation(loop, error));
  const auto loopOps = loop.regions()[0]->blocks()[0]->operations();
  // TODO(lesson 7.3): fails until the tiled generic is vectorized.
  assert(loopOps.size() == 9);
  assert(loopOps[2]->name() == "vector.from_tensor");
  assert(loopOps[3]->name() == "vector.from_tensor");
  assert(loopOps[4]->name() == "vector.add");
  assert(loopOps[5]->name() == "vector.relu");
  assert(loopOps[6]->name() == "vector.to_tensor");
  const TensorData after = mini::interpreter::runFunction(function, inputs);
  assert(after.type == before.type);
  assert(after.elements == before.elements);
  assert(after.elements[16] == 6.0f);
  assert(after.elements[17] == 6.0f);
}
