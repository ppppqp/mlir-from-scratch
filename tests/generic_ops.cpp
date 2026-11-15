#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;
  const Type tensor = Type::tensor({4});
  Value a(tensor);
  Value b(tensor);
  std::string error;

  Operation generic("mini.generic", {&a, &b}, {tensor});
  auto& body = generic.addRegion().addBlock();
  Value& scalarA = body.addArgument(Type::f32());
  Value& scalarB = body.addArgument(Type::f32());
  Operation& sum = body.append(std::make_unique<Operation>(
      "mini.scalar_add", std::vector<Value*>{&scalarA, &scalarB},
      std::vector<Type>{Type::f32()}));
  Operation& relu = body.append(std::make_unique<Operation>(
      "mini.scalar_relu", std::vector<Value*>{&sum.result(0)},
      std::vector<Type>{Type::f32()}));
  body.append(std::make_unique<Operation>(
      "mini.yield", std::vector<Value*>{&relu.result(0)},
      std::vector<Type>{}));
  assert(mini::ir::verifyOperation(generic, error));
  assert(error.empty());

  Operation bad("mini.generic", {&a, &b}, {tensor});
  auto& badBody = bad.addRegion().addBlock();
  badBody.addArgument(Type::f32());
  badBody.addArgument(Type::f32());
  // TODO(lesson 6.4): must fail because the region has no mini.yield.
  assert(!mini::ir::verifyOperation(bad, error));
  assert(!error.empty());

  Operation badScalar("mini.scalar_add", {&scalarA, &a}, {Type::f32()});
  assert(!mini::ir::verifyOperation(badScalar, error));
  assert(!error.empty());

  Value external(Type::f32());
  Operation badScope("mini.generic", {&a, &b}, {tensor});
  auto& badScopeBody = badScope.addRegion().addBlock();
  badScopeBody.addArgument(Type::f32());
  badScopeBody.addArgument(Type::f32());
  badScopeBody.append(std::make_unique<Operation>(
      "mini.yield", std::vector<Value*>{&external}, std::vector<Type>{}));
  assert(!mini::ir::verifyOperation(badScope, error));
  assert(!error.empty());
}
