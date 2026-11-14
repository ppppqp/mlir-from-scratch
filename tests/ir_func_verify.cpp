#include "mini/ir/Attribute.h"
#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace {

std::unique_ptr<mini::ir::Operation> makeFunction(
    const mini::ir::Type& inputType,
    const mini::ir::Type& declaredResultType) {
  using mini::ir::FunctionType;
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  auto function = std::make_unique<Operation>("func.func",
                                               std::vector<Value*>{},
                                               std::vector<Type>{});
  function->setAttribute("function_type",
                         FunctionType{{inputType}, {declaredResultType}});
  auto& block = function->addRegion().addBlock();
  Value& argument = block.addArgument(inputType);
  block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&argument}, std::vector<Type>{}));
  return function;
}

}  // namespace

int main() {
  using mini::ir::FuncOp;
  using mini::ir::Type;

  const Type tensor4 = Type::tensor({4});
  const Type tensor8 = Type::tensor({8});
  std::string error;

  auto valid = makeFunction(tensor4, tensor4);
  assert(FuncOp(*valid).verifyRegions(error));
  assert(error.empty());

  auto mismatchedReturn = makeFunction(tensor4, tensor8);
  assert(!FuncOp(*mismatchedReturn).verifyRegions(error));
  assert(!error.empty());
}
