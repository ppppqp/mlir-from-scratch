#include "mini/ir/Verify.h"

#include "mini/ir/Operation.h"
#include "mini/ops/MiniAddOp.h"
#include "mini/ops/MiniMatmulOp.h"
#include "mini/ops/MiniReluOp.h"

#include <array>
#include <string_view>

namespace mini::ir {
namespace {

template <typename OpView>
bool verifyAs(const Operation& operation, std::string& error) {
  return OpView(operation).verify(error);
}

struct VerifierEntry {
  std::string_view name;
  bool (*verify)(const Operation&, std::string&);
};

constexpr std::array verifiers{
    VerifierEntry{ops::MiniAddOp::name, &verifyAs<ops::MiniAddOp>},
    VerifierEntry{ops::MiniReluOp::name, &verifyAs<ops::MiniReluOp>},
    VerifierEntry{ops::MiniMatmulOp::name, &verifyAs<ops::MiniMatmulOp>},
};

}  // namespace

bool verifyOperation(const Operation& operation, std::string& error) {
  error.clear();
  for (const auto& entry : verifiers) {
    if (operation.name() == entry.name) {
      return entry.verify(operation, error);
    }
  }
  error = "unsupported computation operation: " +
          std::string(operation.name());
  return false;
}

}  // namespace mini::ir
