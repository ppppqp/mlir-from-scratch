#include "mini/analysis/UseCountAnalysis.h"
#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"

#include <cassert>
#include <memory>
#include <vector>

int main() {
  using mini::analysis::UseCountAnalysis;
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor = Type::tensor({4});
  Value outsider(tensor);
  Operation function("func.func", {}, {});
  auto& block = function.addRegion().addBlock();
  Value& a = block.addArgument(tensor);
  Value& b = block.addArgument(tensor);
  Value& unused = block.addArgument(tensor);

  auto add = std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&a, &b}, std::vector<Type>{tensor});
  Value& sum = add->result(0);
  block.append(std::move(add));

  auto relu = std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&sum}, std::vector<Type>{tensor});
  Value& out = relu->result(0);
  block.append(std::move(relu));
  Operation& returnOp = block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&out}, std::vector<Type>{}));

  const UseCountAnalysis first(function);
  assert(first.useCount(a) == 1);
  assert(first.useCount(b) == 1);
  assert(first.useCount(sum) == 1);
  assert(first.useCount(out) == 1);
  assert(first.useCount(unused) == 0);
  assert(!first.useCount(outsider).has_value());

  block.insertBefore(returnOp, std::make_unique<Operation>(
                                   "mini.relu", std::vector<Value*>{&sum},
                                   std::vector<Type>{tensor}));
  Operation& repeatedOperand = block.insertBefore(
      returnOp, std::make_unique<Operation>(
                    "mini.add", std::vector<Value*>{&a, &a},
                    std::vector<Type>{tensor}));
  const UseCountAnalysis second(function);
  assert(second.useCount(sum) == 2);
  assert(second.useCount(a) == 3);
  assert(second.useCount(repeatedOperand.result(0)) == 0);
}
