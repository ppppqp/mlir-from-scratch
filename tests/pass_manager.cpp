#include "mini/ir/Operation.h"
#include "mini/pass/Pass.h"
#include "mini/pass/PassManager.h"

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class RecordingPass final : public mini::pass::Pass {
 public:
  RecordingPass(std::string label, std::vector<std::string>& log,
                bool shouldFail = false)
      : label_(std::move(label)), log_(log), shouldFail_(shouldFail) {}

  std::string_view name() const override { return label_; }

  bool run(mini::ir::Operation& operation, std::string& error) override {
    log_.push_back(label_ + ":" + std::string(operation.name()));
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
}
