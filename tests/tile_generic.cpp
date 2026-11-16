#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/SideEffectInterface.h"
#include "mini/pass/PassManager.h"
#include "mini/pass/TileGenericPass.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;
  const Type full = Type::tensor({18});
  std::string error;

  Operation empty("tensor.empty", {}, {full});
  assert(mini::ir::verifyOperation(empty, error));
  assert(!mini::ops::SideEffectInterface::hasSideEffects(empty));
  Operation invalidEmpty("tensor.empty", {}, {Type::tensor({0})});
  assert(!mini::ir::verifyOperation(invalidEmpty, error));

  Operation function("func.func", {}, {});
  auto& block = function.addRegion().addBlock();
  Value& a = block.addArgument(full);
  Value& b = block.addArgument(full);
  Operation& generic = block.append(std::make_unique<Operation>(
      "mini.generic", std::vector<Value*>{&a, &b}, std::vector<Type>{full}));
  auto& body = generic.addRegion().addBlock();
  Value& scalarA = body.addArgument(Type::f32());
  Value& scalarB = body.addArgument(Type::f32());
  Operation& sum = body.append(std::make_unique<Operation>(
      "mini.scalar_add", std::vector<Value*>{&scalarA, &scalarB},
      std::vector<Type>{Type::f32()}));
  body.append(std::make_unique<Operation>(
      "mini.yield", std::vector<Value*>{&sum.result(0)},
      std::vector<Type>{}));
  Operation& ret = block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&generic.result(0)},
      std::vector<Type>{}));
  assert(mini::ir::verifyOperation(generic, error));

  mini::pass::PassManager manager("func.func");
  manager.addPass(std::make_unique<mini::pass::TileGenericPass>());
  assert(manager.run(function, error));
  // TODO(lesson 7.2): fails until mini.generic is tiled.
  assert(block.operations().size() == 3);
  assert(block.operations()[0]->name() == "tensor.empty");
  const Operation& loop = *block.operations()[1];
  assert(loop.name() == "scf.for");
  assert(mini::ir::verifyOperation(loop, error));
  assert(std::get<std::int64_t>(*loop.findAttribute("upper_bound")) == 18);
  assert(std::get<std::int64_t>(*loop.findAttribute("step")) == 4);
  assert(ret.operands()[0] == &loop.result(0));
  const auto loopOps = loop.regions()[0]->blocks()[0]->operations();
  assert(loopOps.size() == 5);
  assert(loopOps[0]->name() == "tensor.extract_slice");
  assert(loopOps[1]->name() == "tensor.extract_slice");
  assert(loopOps[2]->name() == "mini.generic");
  assert(loopOps[3]->name() == "tensor.insert_slice");
  assert(loopOps[4]->name() == "scf.yield");

  const Type divisible = Type::tensor({16});
  Operation exact("func.func", {}, {});
  auto& exactBlock = exact.addRegion().addBlock();
  Value& exactA = exactBlock.addArgument(divisible);
  Value& exactB = exactBlock.addArgument(divisible);
  Operation& exactGeneric = exactBlock.append(std::make_unique<Operation>(
      "mini.generic", std::vector<Value*>{&exactA, &exactB},
      std::vector<Type>{divisible}));
  auto& exactBody = exactGeneric.addRegion().addBlock();
  Value& exactScalarA = exactBody.addArgument(Type::f32());
  exactBody.addArgument(Type::f32());
  exactBody.append(std::make_unique<Operation>(
      "mini.yield", std::vector<Value*>{&exactScalarA},
      std::vector<Type>{}));
  exactBlock.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&exactGeneric.result(0)},
      std::vector<Type>{}));
  assert(manager.run(exact, error));
  assert(exactBlock.operations().size() == 3);
  assert(std::get<std::int64_t>(
             *exactBlock.operations()[1]->findAttribute("upper_bound")) == 16);
}
