#include "mini/ir/Attribute.h"
#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace {

std::unique_ptr<mini::ir::Operation> makeFunction(
    std::string name, mini::ir::Type leftType, mini::ir::Type rightType,
    mini::ir::Type resultType) {
  using mini::ir::FunctionType;
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  auto function = std::make_unique<Operation>("func.func",
                                               std::vector<Value*>{},
                                               std::vector<Type>{});
  function->setAttribute("sym_name", std::move(name));
  function->setAttribute("function_type",
                         FunctionType{{leftType, rightType}, {resultType}});
  auto& body = function->addRegion().addBlock();
  Value& left = body.addArgument(leftType);
  Value& right = body.addArgument(rightType);
  auto add = std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&left, &right},
      std::vector<Type>{resultType});
  Value* sum = &add->result(0);
  body.append(std::move(add));
  body.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{sum}, std::vector<Type>{}));
  return function;
}

std::unique_ptr<mini::ir::Operation> makeModule() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  auto module = std::make_unique<Operation>("builtin.module",
                                             std::vector<Value*>{},
                                             std::vector<Type>{});
  module->addRegion().addBlock();
  return module;
}

}  // namespace

int main() {
  using mini::ir::Type;

  const Type tensor4 = Type::tensor({4});
  const Type tensor8 = Type::tensor({8});
  std::string error;

  auto valid = makeModule();
  valid->regions()[0]->blocks()[0]->append(
      makeFunction("compute", tensor4, tensor4, tensor4));
  assert(mini::ir::verifyModule(*valid, error));
  assert(error.empty());

  auto badAdd = makeModule();
  badAdd->regions()[0]->blocks()[0]->append(
      makeFunction("bad", tensor4, tensor8, tensor4));
  assert(!mini::ir::verifyModule(*badAdd, error));
  assert(error.find("@bad") != std::string::npos);
  assert(error.find("mini.add") != std::string::npos);

  auto badReturn = makeModule();
  auto wrongSignature = makeFunction("returns_wrong_type", tensor4, tensor4,
                                     tensor4);
  wrongSignature->setAttribute(
      "function_type",
      mini::ir::FunctionType{{tensor4, tensor4}, {tensor8}});
  badReturn->regions()[0]->blocks()[0]->append(std::move(wrongSignature));
  assert(!mini::ir::verifyModule(*badReturn, error));
  assert(error.find("func.return") != std::string::npos);

  mini::ir::Value foreign(tensor4);
  auto badScope = makeModule();
  auto usesForeign = makeFunction("outside", tensor4, tensor4, tensor4);
  auto& add = *usesForeign->regions()[0]->blocks()[0]->operations()[0];
  add.setOperand(0, &foreign);
  badScope->regions()[0]->blocks()[0]->append(std::move(usesForeign));
  assert(!mini::ir::verifyModule(*badScope, error));
  assert(error.find("outside this block") != std::string::npos);

  auto duplicateNames = makeModule();
  auto& functions = *duplicateNames->regions()[0]->blocks()[0];
  functions.append(makeFunction("same", tensor4, tensor4, tensor4));
  functions.append(makeFunction("same", tensor4, tensor4, tensor4));
  assert(!mini::ir::verifyModule(*duplicateNames, error));
  assert(error.find("duplicate function symbol") != std::string::npos);
}
