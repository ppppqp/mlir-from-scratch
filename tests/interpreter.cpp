#include "mini/interpreter/Interpreter.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/pass/FuseAddReluPass.h"
#include "mini/pass/PassManager.h"

#include <string>

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

  const std::vector<TensorData> inputs{{tensor4, {-1.0f, 2.0f, -3.0f, 4.0f}},
                                       {tensor4, {0.0f, -3.0f, 5.0f, 1.0f}}};
  const TensorData result = mini::interpreter::runFunction(function, inputs);
  assert(result.type == tensor4);
  assert((result.elements == std::vector<float>{0.0f, 0.0f, 2.0f, 5.0f}));
  assert((inputs[0].elements == std::vector<float>{-1.0f, 2.0f, -3.0f, 4.0f}));

  mini::pass::PassManager fusion("func.func");
  fusion.addPass(std::make_unique<mini::pass::FuseAddReluPass>());
  std::string fusionError;
  assert(fusion.run(function, fusionError));
  assert(block.operations().size() == 2);
  const TensorData fused = mini::interpreter::runFunction(function, inputs);
  assert(fused.type == result.type);
  assert(fused.elements == result.elements);

  auto shortInputs = inputs;
  shortInputs[0].elements.pop_back();
  bool rejected = false;
  try {
    (void)mini::interpreter::runFunction(function, shortInputs);
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  assert(rejected);

  const Type matrix16 = Type::tensor({16, 16});
  Operation matmulFunction("func.func", {}, {});
  auto& matmulBlock = matmulFunction.addRegion().addBlock();
  Value& lhs = matmulBlock.addArgument(matrix16);
  Value& rhs = matmulBlock.addArgument(matrix16);
  auto matmul = std::make_unique<Operation>("mini.matmul",
                                            std::vector<Value*>{&lhs, &rhs},
                                            std::vector<Type>{matrix16});
  Value& product = matmul->result(0);
  matmulBlock.append(std::move(matmul));
  matmulBlock.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&product}, std::vector<Type>{}));

  std::vector<TensorData> matrices{{matrix16, std::vector<float>(256, 0.0f)},
                                   {matrix16, std::vector<float>(256, 0.0f)}};
  matrices[0].elements[1 * 16 + 3] = 2.0f;
  matrices[1].elements[3 * 16 + 2] = 4.0f;
  matrices[0].elements[1 * 16 + 5] = -1.0f;
  matrices[1].elements[5 * 16 + 2] = 3.0f;
  const TensorData productData =
      mini::interpreter::runFunction(matmulFunction, matrices);
  assert(productData.type == matrix16);
  for (std::size_t i = 0; i < productData.elements.size(); ++i) {
    assert(productData.elements[i] == (i == 1 * 16 + 2 ? 5.0f : 0.0f));
  }
  assert(matrices[0].elements[1 * 16 + 3] == 2.0f);

  Operation dpsFunction("func.func", {}, {});
  auto& dpsBlock = dpsFunction.addRegion().addBlock();
  Value& dpsA = dpsBlock.addArgument(tensor4);
  Value& dpsB = dpsBlock.addArgument(tensor4);
  Value& init = dpsBlock.addArgument(tensor4);
  auto dpsAdd = std::make_unique<Operation>(
      "mini.dps_add", std::vector<Value*>{&dpsA, &dpsB, &init},
      std::vector<Type>{tensor4});
  Value& dpsSum = dpsAdd->result(0);
  dpsBlock.append(std::move(dpsAdd));
  auto dpsRelu = std::make_unique<Operation>(
      "mini.dps_relu", std::vector<Value*>{&dpsSum, &init},
      std::vector<Type>{tensor4});
  Value& dpsOut = dpsRelu->result(0);
  dpsBlock.append(std::move(dpsRelu));
  Operation& dpsReturn = dpsBlock.append(std::make_unique<Operation>(
      "func.return", std::vector<Value*>{&dpsOut}, std::vector<Type>{}));

  const std::vector<TensorData> dpsInputs{
      {tensor4, {1.0f, 2.0f, 3.0f, 4.0f}},
      {tensor4, {5.0f, 6.0f, 7.0f, 8.0f}},
      {tensor4, {100.0f, 100.0f, 100.0f, 100.0f}}};
  const TensorData dpsResult =
      mini::interpreter::runFunction(dpsFunction, dpsInputs);
  assert((dpsResult.elements == std::vector<float>{6.0f, 8.0f, 10.0f, 12.0f}));
  assert((dpsInputs[2].elements ==
          std::vector<float>{100.0f, 100.0f, 100.0f, 100.0f}));

  dpsReturn.setOperand(0, &init);
  const TensorData oldInit =
      mini::interpreter::runFunction(dpsFunction, dpsInputs);
  assert(
      (oldInit.elements == std::vector<float>{100.0f, 100.0f, 100.0f, 100.0f}));
}
