#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/pass/FuseAddReluPass.h"
#include "mini/pass/PassManager.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;
  const Type tensor = Type::tensor({4});
  mini::pass::PassManager manager("func.func");
  manager.addPass(std::make_unique<mini::pass::FuseAddReluPass>());
  std::string error;

  Operation function("func.func", {}, {});
  auto& block = function.addRegion().addBlock();
  Value& a = block.addArgument(tensor);
  Value& b = block.addArgument(tensor);
  Operation& add = block.append(std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&a, &b}, std::vector<Type>{tensor}));
  Operation& relu = block.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&add.result(0)},
      std::vector<Type>{tensor}));
  Operation& ret = block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&relu.result(0)},
      std::vector<Type>{}));
  assert(manager.run(function, error));
  assert(error.empty());
  // TODO(lesson 6.5): fails until the fusion rewrite is implemented.
  assert(block.operations().size() == 2);
  const Operation& generic = *block.operations()[0];
  assert(generic.name() == "mini.generic");
  assert(generic.operands()[0] == &a && generic.operands()[1] == &b);
  assert(mini::ir::verifyOperation(generic, error));
  assert(ret.operands()[0] == &generic.result(0));
  const auto body = generic.regions()[0]->blocks()[0]->operations();
  assert(body.size() == 3);
  assert(body[0]->name() == "mini.scalar_add");
  assert(body[1]->name() == "mini.scalar_relu");
  assert(body[2]->name() == "mini.yield");

  Operation shared("func.func", {}, {});
  auto& sharedBlock = shared.addRegion().addBlock();
  Value& sharedA = sharedBlock.addArgument(tensor);
  Value& sharedB = sharedBlock.addArgument(tensor);
  Operation& sharedAdd = sharedBlock.append(std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&sharedA, &sharedB},
      std::vector<Type>{tensor}));
  sharedBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&sharedAdd.result(0)},
      std::vector<Type>{tensor}));
  Operation& otherUse = sharedBlock.append(std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&sharedAdd.result(0), &sharedA},
      std::vector<Type>{tensor}));
  sharedBlock.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&otherUse.result(0)},
      std::vector<Type>{}));
  assert(manager.run(shared, error));
  assert(sharedBlock.operations().size() == 4);
  assert(sharedBlock.operations()[0]->name() == "mini.add");
  assert(sharedAdd.result(0).useCount() == 2);
}
