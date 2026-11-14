#include "mini/ir/Attribute.h"
#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Parser.h"
#include "mini/ir/Printer.h"
#include "mini/ir/Region.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

int main() {
  using mini::ir::Attribute;
  using mini::ir::FunctionType;
  using mini::ir::Operation;
  using mini::ir::Type;

  const Type tensor = Type::tensor({4});
  Operation function("func.func", {}, {});
  function.setAttribute("sym_name", std::string("compute"));
  function.setAttribute("function_type",
                        FunctionType{{tensor, tensor}, {tensor}});
  assert(function.resultCount() == 0);
  const Attribute* name = function.findAttribute("sym_name");
  assert(name != nullptr);
  assert(std::get<std::string>(*name) == "compute");
  const Attribute* signature = function.findAttribute("function_type");
  assert(signature != nullptr);
  assert(std::get<FunctionType>(*signature) ==
         (FunctionType{{tensor, tensor}, {tensor}}));
  assert(function.findAttribute("missing") == nullptr);

  constexpr std::string_view source = R"mlir(
module {
  func.func @compute(%A: tensor<4xf32>, %B: tensor<4xf32>) -> tensor<4xf32> {
    %sum = "mini.add"(%A, %B) : (tensor<4xf32>, tensor<4xf32>) -> tensor<4xf32>
    %out = "mini.relu"(%sum) : (tensor<4xf32>) -> tensor<4xf32>
    func.return %out : tensor<4xf32>
  }
}
)mlir";

  std::unique_ptr<Operation> module = mini::ir::parseModule(source);
  assert(module != nullptr);
  assert(module->name() == "builtin.module");
  const auto& moduleBlock = *module->regions()[0]->blocks()[0];
  assert(moduleBlock.operations().size() == 1);
  const Operation& parsedFunction = *moduleBlock.operations()[0];
  assert(parsedFunction.name() == "func.func");
  assert(parsedFunction.resultCount() == 0);
  assert(std::get<std::string>(*parsedFunction.findAttribute("sym_name")) ==
         "compute");
  const auto& entry = *parsedFunction.regions()[0]->blocks()[0];
  assert(entry.arguments().size() == 2);
  assert(entry.operations().size() == 3);
  assert(entry.operations()[0]->name() == "mini.add");
  assert(entry.operations()[1]->name() == "mini.relu");
  assert(entry.operations()[2]->name() == "func.return");
  assert(entry.operations()[1]->operands()[0] ==
         &entry.operations()[0]->result(0));
  assert(entry.operations()[2]->operands()[0] ==
         &entry.operations()[1]->result(0));

  const std::string printed = mini::ir::printModule(*module);
  assert(printed.find("@compute") != std::string::npos);
  assert(printed.find("tensor<4xf32>") != std::string::npos);
  std::unique_ptr<Operation> again = mini::ir::parseModule(printed);
  assert(mini::ir::printModule(*again) == printed);

  constexpr std::string_view twoFunctions = R"mlir(
module {
  // Matmul uses rank-2 tensors.
  func.func @matmul(%lhs: tensor<16x32xf32>, %rhs: tensor<32x16xf32>) -> tensor<16x16xf32> {
    %product = "mini.matmul"(%lhs, %rhs) : (tensor<16x32xf32>, tensor<32x16xf32>) -> tensor<16x16xf32>
    func.return %product : tensor<16x16xf32>
  }
  func.func @identity(%input: tensor<4xf32>) -> tensor<4xf32> {
    func.return %input : tensor<4xf32>
  }
}
)mlir";
  std::unique_ptr<Operation> multi = mini::ir::parseModule(twoFunctions);
  const std::string multiPrinted = mini::ir::printModule(*multi);
  assert(multiPrinted.find("@matmul") != std::string::npos);
  assert(multiPrinted.find("@identity") != std::string::npos);
  assert(mini::ir::printModule(*mini::ir::parseModule(multiPrinted)) ==
         multiPrinted);

  constexpr std::string_view undefinedValue = R"mlir(
module {
  func.func @bad(%input: tensor<4xf32>) -> tensor<4xf32> {
    func.return %missing : tensor<4xf32>
  }
}
)mlir";
  bool rejected = false;
  try {
    (void)mini::ir::parseModule(undefinedValue);
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  assert(rejected);
}
