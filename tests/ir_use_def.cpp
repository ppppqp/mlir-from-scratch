#include "mini/ir/Operation.h"

#include <cassert>
#include <memory>
#include <stdexcept>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor = Type::tensor({4});
  assert(tensor == Type::tensor({4}));
  assert(!(tensor == Type::tensor({8})));

  Value a(tensor);
  Value b(tensor);
  auto add = std::make_unique<Operation>(
      "add", std::vector<Value*>{&a, &b}, std::vector<Type>{tensor});
  auto relu = std::make_unique<Operation>(
      "relu", std::vector<Value*>{&add->result(0)}, std::vector<Type>{tensor});

  assert(a.useCount() == 1);
  assert(b.useCount() == 1);
  assert(add->result(0).definingOp() == add.get());
  assert(add->result(0).resultIndex() == 0);
  assert(add->result(0).useCount() == 1);
  assert(add->result(0).uses()[0].owner == relu.get());
  assert(add->result(0).uses()[0].operandIndex == 0);

  relu->setOperand(0, &a);
  assert(add->result(0).useCount() == 0);
  assert(a.useCount() == 2);
  assert(relu->operands()[0] == &a);

  relu.reset();
  assert(a.useCount() == 1);
  add.reset();
  assert(a.useCount() == 0);
  assert(b.useCount() == 0);

  // A failed constructor must not leave a use pointing to a nonexistent op.
  bool rejectedNullOperand = false;
  try {
    Operation invalid("invalid", {&a, nullptr}, {});
  } catch (const std::invalid_argument&) {
    rejectedNullOperand = true;
  }
  assert(rejectedNullOperand);
  assert(a.useCount() == 0);
}
