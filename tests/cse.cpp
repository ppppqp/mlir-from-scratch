#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/pass/CsePass.h"
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
  manager.addPass(std::make_unique<mini::pass::CsePass>());
  std::string error;

  Operation duplicate("func.func", {}, {});
  auto& block = duplicate.addRegion().addBlock();
  Value& input = block.addArgument(tensor);
  Operation& first = block.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&input}, std::vector<Type>{tensor}));
  Operation& second = block.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&input}, std::vector<Type>{tensor}));
  Operation& add = block.append(std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&first.result(0), &second.result(0)},
      std::vector<Type>{tensor}));
  block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&add.result(0)},
      std::vector<Type>{}));
  assert(manager.run(duplicate, error));
  assert(error.empty());
  // TODO(lesson 6.3): duplicate relu is eliminated.
  assert(block.operations().size() == 3);
  assert(add.operands()[0] == &first.result(0));
  assert(add.operands()[1] == &first.result(0));
  assert(first.result(0).useCount() == 2);

  Operation distinct("func.func", {}, {});
  auto& distinctBlock = distinct.addRegion().addBlock();
  Value& left = distinctBlock.addArgument(tensor);
  Value& right = distinctBlock.addArgument(tensor);
  Operation& leftRelu = distinctBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&left}, std::vector<Type>{tensor}));
  Operation& rightRelu = distinctBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&right}, std::vector<Type>{tensor}));
  Operation& distinctAdd = distinctBlock.append(std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&leftRelu.result(0),
                                      &rightRelu.result(0)},
      std::vector<Type>{tensor}));
  distinctBlock.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&distinctAdd.result(0)},
      std::vector<Type>{}));
  assert(manager.run(distinct, error));
  assert(distinctBlock.operations().size() == 4);
  assert(distinctAdd.operands()[0] != distinctAdd.operands()[1]);

  Operation repeated("func.func", {}, {});
  auto& repeatedBlock = repeated.addRegion().addBlock();
  Value& repeatedInput = repeatedBlock.addArgument(tensor);
  Operation& kept = repeatedBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&repeatedInput},
      std::vector<Type>{tensor}));
  repeatedBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&repeatedInput},
      std::vector<Type>{tensor}));
  Operation& last = repeatedBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&repeatedInput},
      std::vector<Type>{tensor}));
  Operation& repeatedReturn = repeatedBlock.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&last.result(0)},
      std::vector<Type>{}));
  assert(manager.run(repeated, error));
  assert(repeatedBlock.operations().size() == 2);
  assert(repeatedReturn.operands()[0] == &kept.result(0));
}
