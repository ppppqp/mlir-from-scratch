#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/pass/CanonicalizePass.h"
#include "mini/pass/PassManager.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  Operation function("func.func", {}, {});
  auto& block = function.addRegion().addBlock();
  const Type tensor = Type::tensor({4});
  Value& input = block.addArgument(tensor);
  Operation& inner = block.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&input}, std::vector<Type>{tensor}));
  Operation& outer = block.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&inner.result(0)},
      std::vector<Type>{tensor}));
  Operation& ret = block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&outer.result(0)},
      std::vector<Type>{}));

  mini::pass::PassManager manager("func.func");
  manager.addPass(std::make_unique<mini::pass::CanonicalizePass>());
  std::string error;
  assert(manager.run(function, error));
  assert(error.empty());

  // TODO(lesson 6.1): these assertions fail until the rewrite is implemented.
  assert(block.operations().size() == 2);
  assert(block.operations()[0].get() == &inner);
  assert(block.operations()[1].get() == &ret);
  assert(ret.operands()[0] == &inner.result(0));
  assert(inner.result(0).useCount() == 1);
  assert(input.useCount() == 1);

  Operation chain("func.func", {}, {});
  auto& chainBlock = chain.addRegion().addBlock();
  Value& chainInput = chainBlock.addArgument(tensor);
  Operation& first = chainBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&chainInput}, std::vector<Type>{tensor}));
  Operation& second = chainBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&first.result(0)},
      std::vector<Type>{tensor}));
  Operation& third = chainBlock.append(std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&second.result(0)},
      std::vector<Type>{tensor}));
  Operation& chainReturn = chainBlock.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&third.result(0)},
      std::vector<Type>{}));
  assert(manager.run(chain, error));
  assert(chainBlock.operations().size() == 2);
  assert(chainBlock.operations()[0].get() == &first);
  assert(chainReturn.operands()[0] == &first.result(0));
}
