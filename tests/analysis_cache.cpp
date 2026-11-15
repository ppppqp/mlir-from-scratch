#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/pass/PassManager.h"

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace {

class RewriteOperand final : public mini::pass::Pass {
 public:
  std::string_view name() const override { return "rewrite-operand"; }
  bool run(mini::ir::Operation& function, std::string&) override {
    auto& block = *function.regions()[0]->blocks()[0];
    block.operations()[0]->setOperand(1, block.arguments()[0].get());
    return true;
  }
};

class ReadOnlyPass final : public mini::pass::Pass {
 public:
  std::string_view name() const override { return "read-only"; }
  bool preservesUseCountAnalysis() const override { return true; }
  bool run(mini::ir::Operation&, std::string&) override { return true; }
};

class FailAfterRewrite final : public mini::pass::Pass {
 public:
  std::string_view name() const override { return "fail-after-rewrite"; }
  bool run(mini::ir::Operation& function, std::string& error) override {
    auto& block = *function.regions()[0]->blocks()[0];
    block.operations()[0]->setOperand(1, block.arguments()[1].get());
    error = "deliberate failure after mutation";
    return false;
  }
};

}  // namespace

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor = Type::tensor({4});
  Operation function("func.func", {}, {});
  auto& block = function.addRegion().addBlock();
  Value& a = block.addArgument(tensor);
  Value& b = block.addArgument(tensor);
  block.append(std::make_unique<Operation>(
      "mini.add", std::vector<Value*>{&a, &b}, std::vector<Type>{tensor}));

  mini::pass::PassManager manager("func.func");
  auto& cache = manager.analyses();
  const auto& initial = cache.getUseCount(function);
  assert(initial.useCount(a) == 1);
  assert(initial.useCount(b) == 1);
  assert(&cache.getUseCount(function) == &initial);

  manager.addPass(std::make_unique<ReadOnlyPass>());
  std::string error;
  assert(manager.run(function, error));
  assert(&cache.getUseCount(function) == &initial);

  manager.addPass(std::make_unique<RewriteOperand>());
  assert(manager.run(function, error));
  const auto& updated = cache.getUseCount(function);
  assert(updated.useCount(a) == 2);
  assert(updated.useCount(b) == 0);

  mini::pass::PassManager failing("func.func");
  auto& failingCache = failing.analyses();
  assert(failingCache.getUseCount(function).useCount(a) == 2);
  failing.addPass(std::make_unique<FailAfterRewrite>());
  assert(!failing.run(function, error));
  assert(failingCache.getUseCount(function).useCount(a) == 1);
  assert(failingCache.getUseCount(function).useCount(b) == 1);
}
