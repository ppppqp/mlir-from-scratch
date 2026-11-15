#include "mini/ir/Operation.h"
#include "mini/ir/Parser.h"
#include "mini/ir/Printer.h"
#include "mini/ir/Verify.h"
#include "mini/ops/MiniDpsAddOp.h"
#include "mini/ops/MiniDpsReluOp.h"

#include <cassert>
#include <string>
#include <string_view>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor4 = Type::tensor({4});
  const Type tensor8 = Type::tensor({8});
  Value a(tensor4);
  Value b(tensor4);
  Value init(tensor4);
  Value wrongInit(tensor8);
  std::string error;

  {
    Operation add("mini.dps_add", {&a, &b, &init}, {tensor4});
    assert(&add.result(0) != &init);
    assert(mini::ops::MiniDpsAddOp(add).verify(error));
    assert(error.empty());
  }
  {
    Operation relu("mini.dps_relu", {&a, &init}, {tensor4});
    assert(&relu.result(0) != &init);
    assert(mini::ops::MiniDpsReluOp(relu).verify(error));
    assert(error.empty());
  }
  {
    Operation badAdd("mini.dps_add", {&a, &b, &wrongInit}, {tensor4});
    assert(!mini::ops::MiniDpsAddOp(badAdd).verify(error));
    assert(!error.empty());
  }
  {
    Operation badRelu("mini.dps_relu", {&a, &init}, {tensor8});
    assert(!mini::ops::MiniDpsReluOp(badRelu).verify(error));
    assert(!error.empty());
  }

  constexpr std::string_view source = R"mlir(
module {
  func.func @compute(%a: tensor<4xf32>, %b: tensor<4xf32>, %init: tensor<4xf32>) -> tensor<4xf32> {
    %sum = "mini.dps_add"(%a, %b, %init) : (tensor<4xf32>, tensor<4xf32>, tensor<4xf32>) -> tensor<4xf32>
    %out = "mini.dps_relu"(%sum, %init) : (tensor<4xf32>, tensor<4xf32>) -> tensor<4xf32>
    func.return %out : tensor<4xf32>
  }
}
)mlir";
  auto module = mini::ir::parseModule(source);
  assert(mini::ir::verifyModule(*module, error));
  const std::string printed = mini::ir::printModule(*module);
  assert(mini::ir::printModule(*mini::ir::parseModule(printed)) == printed);
}
