#include "mini/ir/Block.h"
#include "mini/ir/Region.h"
#include "mini/ir/Walk.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

int main() {
  using mini::ir::Block;
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor = Type::tensor({4});
  Block block;
  Value& a = block.addArgument(tensor);
  Value& b = block.addArgument(tensor);

  auto add = std::make_unique<Operation>(
      "add", std::vector<Value*>{&a, &b}, std::vector<Type>{tensor});
  Value& sum = add->result(0);
  Operation& addOp = block.append(std::move(add));
  Operation& x = block.append(std::make_unique<Operation>(
      "relu.x", std::vector<Value*>{&sum}, std::vector<Type>{tensor}));
  Operation& y = block.append(std::make_unique<Operation>(
      "relu.y", std::vector<Value*>{&sum}, std::vector<Type>{tensor}));

  bool rejectedLiveResult = false;
  try {
    block.erase(addOp);
  } catch (const std::logic_error&) {
    rejectedLiveResult = true;
  }
  assert(rejectedLiveResult);

  sum.replaceAllUsesWith(a);
  assert(sum.useCount() == 0);
  assert(a.useCount() == 3);
  assert(x.operands()[0] == &a);
  assert(y.operands()[0] == &a);

  block.erase(addOp);
  assert(a.useCount() == 2);
  assert(b.useCount() == 0);

  Operation& marker = block.insertBefore(
      y, std::make_unique<Operation>("marker", std::vector<Value*>{},
                                      std::vector<Type>{}));
  assert(marker.name() == "marker");
  assert(block.operations().size() == 3);
  assert(block.operations()[0].get() == &x);
  assert(block.operations()[1].get() == &marker);
  assert(block.operations()[2].get() == &y);

  auto root = std::make_unique<Operation>(
      "builtin.module", std::vector<Value*>{}, std::vector<Type>{});
  Block& moduleBody = root->addRegion().addBlock();
  auto function = std::make_unique<Operation>(
      "func.func", std::vector<Value*>{}, std::vector<Type>{});
  function->addRegion().addBlock().append(std::make_unique<Operation>(
      "leaf", std::vector<Value*>{}, std::vector<Type>{}));
  moduleBody.append(std::move(function));

  std::vector<std::string_view> observed;
  mini::ir::walk(*root, [&](Operation& op) { observed.push_back(op.name()); });
  const std::vector<std::string_view> expected = {"builtin.module", "func.func",
                                                  "leaf"};
  assert(observed == expected);
}
