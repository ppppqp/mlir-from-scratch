#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/pass/Pass.h"
#include "mini/pass/PassManager.h"

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <variant>

namespace {

class RecordingPass final : public mini::pass::Pass {
 public:
  RecordingPass(std::string label, std::vector<std::string>& log,
                bool shouldFail = false)
      : label_(std::move(label)), log_(log), shouldFail_(shouldFail) {}

  std::string_view name() const override { return label_; }

  bool run(mini::ir::Operation& operation, std::string& error) override {
    std::string target(operation.name());
    if (const auto* attribute = operation.findAttribute("sym_name")) {
      if (const auto* symbol = std::get_if<std::string>(attribute)) {
        target = "@" + *symbol;
      }
    }
    log_.push_back(label_ + ":" + target);
    if (shouldFail_) {
      error = "deliberate failure";
      return false;
    }
    return true;
  }

 private:
  std::string label_;
  std::vector<std::string>& log_;
  bool shouldFail_;
};

}  // namespace

int main() {
  mini::ir::Operation module("builtin.module", {}, {});
  mini::ir::Operation function("func.func", {}, {});
  std::vector<std::string> log;
  std::string error;

  mini::pass::PassManager ordered("builtin.module");
  ordered.addPass(std::make_unique<RecordingPass>("A", log));
  ordered.addPass(std::make_unique<RecordingPass>("B", log));
  assert(ordered.run(module, error));
  assert(error.empty());
  assert((log == std::vector<std::string>{"A:builtin.module",
                                          "B:builtin.module"}));

  log.clear();
  mini::pass::PassManager failing("builtin.module");
  failing.addPass(std::make_unique<RecordingPass>("A", log, true));
  failing.addPass(std::make_unique<RecordingPass>("B", log));
  assert(!failing.run(module, error));
  assert((log == std::vector<std::string>{"A:builtin.module"}));
  assert(error.find("deliberate failure") != std::string::npos);
  assert(error.find("A") != std::string::npos);

  log.clear();
  assert(!ordered.run(function, error));
  assert(log.empty());
  assert(error.find("func.func") != std::string::npos);

  mini::ir::Operation nestedModule("builtin.module", {}, {});
  auto& body = nestedModule.addRegion().addBlock();
  auto first = std::make_unique<mini::ir::Operation>("func.func",
                                                      std::vector<mini::ir::Value*>{},
                                                      std::vector<mini::ir::Type>{});
  first->setAttribute("sym_name", std::string("f"));
  body.append(std::move(first));
  auto second = std::make_unique<mini::ir::Operation>("func.func",
                                                       std::vector<mini::ir::Value*>{},
                                                       std::vector<mini::ir::Type>{});
  second->setAttribute("sym_name", std::string("g"));
  body.append(std::move(second));

  log.clear();
  mini::pass::PassManager nestedPipeline("builtin.module");
  nestedPipeline.addPass(std::make_unique<RecordingPass>("M1", log));
  auto& functions = nestedPipeline.nest("func.func");
  functions.addPass(std::make_unique<RecordingPass>("F", log));
  nestedPipeline.addPass(std::make_unique<RecordingPass>("M2", log));
  assert(nestedPipeline.run(nestedModule, error));
  assert(error.empty());
  assert((log == std::vector<std::string>{"M1:builtin.module", "F:@f",
                                          "F:@g", "M2:builtin.module"}));

  log.clear();
  mini::pass::PassManager nestedFailure("builtin.module");
  nestedFailure.addPass(std::make_unique<RecordingPass>("M1", log));
  nestedFailure.nest("func.func")
      .addPass(std::make_unique<RecordingPass>("F", log, true));
  nestedFailure.addPass(std::make_unique<RecordingPass>("M2", log));
  assert(!nestedFailure.run(nestedModule, error));
  assert((log == std::vector<std::string>{"M1:builtin.module", "F:@f"}));
  assert(error.find("deliberate failure") != std::string::npos);
}
