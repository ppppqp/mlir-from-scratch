#include "mini/interpreter/Interpreter.h"
#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

int main() {
  using mini::interpreter::TensorData;
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor4 = Type::tensor({4});
  Operation function("func.func", {}, {});
  auto& block = function.addRegion().addBlock();
  Value& a = block.addArgument(tensor4);
  Value& b = block.addArgument(tensor4);

  auto add = std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&a, &b}, std::vector<Type>{tensor4});
  Value& sum = add->result(0);
  block.append(std::move(add));
  auto relu = std::make_unique<Operation>(
      "mini.relu", std::vector<Value*>{&sum}, std::vector<Type>{tensor4});
  Value& out = relu->result(0);
  block.append(std::move(relu));
  block.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&out}, std::vector<Type>{}));

  const std::vector<TensorData> inputs{
      {tensor4, {-1.0f, 2.0f, -3.0f, 4.0f}},
      {tensor4, {0.0f, -3.0f, 5.0f, 1.0f}}};
  const TensorData result = mini::interpreter::runFunction(function, inputs);
  assert(result.type == tensor4);
  assert((result.elements == std::vector<float>{0.0f, 0.0f, 2.0f, 5.0f}));
  assert((inputs[0].elements == std::vector<float>{-1.0f, 2.0f, -3.0f, 4.0f}));

  auto shortInputs = inputs;
  shortInputs[0].elements.pop_back();
  bool rejected = false;
  try {
    (void)mini::interpreter::runFunction(function, shortInputs);
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  assert(rejected);
}
