#include <cassert>
#include <memory>
#include <string_view>
#include <vector>

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/ModuleOp.h"
#include "mini/ir/Region.h"

int main() {
  using mini::ir::Block;
  using mini::ir::FuncOp;
  using mini::ir::ModuleOp;
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor = Type::tensor({4});

  auto root = std::make_unique<Operation>(
      "builtin.module", std::vector<Value*>{}, std::vector<Type>{});
  Block& moduleBody = root->addRegion().addBlock();
  ModuleOp module(*root);
  assert(&module.body() == &moduleBody);

  auto functionNode = std::make_unique<Operation>(
      "func.func", std::vector<Value*>{}, std::vector<Type>{});
  Block& entry = functionNode->addRegion().addBlock();
  Operation& functionOperation = moduleBody.append(std::move(functionNode));
  FuncOp function(functionOperation);
  assert(&function.entryBlock() == &entry);

  Value& a = entry.addArgument(tensor);
  Value& b = entry.addArgument(tensor);
  assert(a.isBlockArgument());
  assert(a.definingOp() == nullptr);
  assert(a.definingBlock() == &entry);
  assert(a.argumentIndex() == 0);
  assert(b.argumentIndex() == 1);

  auto add = std::make_unique<Operation>("add", std::vector<Value*>{&a, &b},
                                         std::vector<Type>{tensor});
  Value* sum = &add->result(0);
  entry.append(std::move(add));

  auto relu = std::make_unique<Operation>("relu", std::vector<Value*>{sum},
                                          std::vector<Type>{tensor});
  Value* output = &relu->result(0);
  entry.append(std::move(relu));
  entry.append(std::make_unique<Operation>(
      "return", std::vector<Value*>{output}, std::vector<Type>{}));

  assert(root->regions().size() == 1);
  assert(root->regions()[0]->blocks().size() == 1);
  assert(moduleBody.operations().size() == 1);
  assert(moduleBody.operations()[0]->name() == "func.func");
  assert(functionOperation.regions().size() == 1);
  assert(entry.arguments().size() == 2);
  assert(entry.operations().size() == 3);
  assert(a.useCount() == 1);
  assert(b.useCount() == 1);
  assert(sum->useCount() == 1);
  assert(output->useCount() == 1);

  const std::vector<std::string_view> expected = {"add", "relu", "return"};
  std::vector<std::string_view> observed;
  for (const auto& operation : function.entryBlock().operations()) {
    observed.push_back(operation->name());
  }
  assert(observed == expected);

  // Destroying root recursively removes users before definitions.
}
