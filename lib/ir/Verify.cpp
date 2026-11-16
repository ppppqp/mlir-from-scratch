#include "mini/ir/Verify.h"

#include <array>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ops/MiniAddOp.h"
#include "mini/ops/MiniDpsAddOp.h"
#include "mini/ops/MiniDpsReluOp.h"
#include "mini/ops/MiniGenericOp.h"
#include "mini/ops/MiniMatmulOp.h"
#include "mini/ops/MiniReluOp.h"
#include "mini/ops/MiniScalarAddOp.h"
#include "mini/ops/MiniScalarReluOp.h"
#include "mini/ops/MiniYieldOp.h"
#include "mini/ops/TilingOps.h"
#include "mini/ops/TensorEmptyOp.h"

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
    VerifierEntry{ops::MiniDpsAddOp::name, &verifyAs<ops::MiniDpsAddOp>},
    VerifierEntry{ops::MiniDpsReluOp::name, &verifyAs<ops::MiniDpsReluOp>},
    VerifierEntry{ops::MiniGenericOp::name, &verifyAs<ops::MiniGenericOp>},
    VerifierEntry{ops::MiniScalarAddOp::name, &verifyAs<ops::MiniScalarAddOp>},
    VerifierEntry{ops::MiniScalarReluOp::name, &verifyAs<ops::MiniScalarReluOp>},
    VerifierEntry{ops::MiniYieldOp::name, &verifyAs<ops::MiniYieldOp>},
    VerifierEntry{ops::ScfForOp::name, &verifyAs<ops::ScfForOp>},
    VerifierEntry{ops::ScfYieldOp::name, &verifyAs<ops::ScfYieldOp>},
    VerifierEntry{ops::TensorExtractSliceOp::name,
                  &verifyAs<ops::TensorExtractSliceOp>},
    VerifierEntry{ops::TensorInsertSliceOp::name,
                  &verifyAs<ops::TensorInsertSliceOp>},
    VerifierEntry{ops::TensorEmptyOp::name, &verifyAs<ops::TensorEmptyOp>},
};

}  // namespace

bool verifyOperation(const Operation& operation, std::string& error) {
  error.clear();
  for (const auto& entry : verifiers) {
    if (operation.name() == entry.name) {
      return entry.verify(operation, error);
    }
  }
  error = "unsupported computation operation: " + std::string(operation.name());
  return false;
}

bool verifyModule(Operation& module, std::string& error) {
  error.clear();
  if (module.name() != "builtin.module" || !module.operands().empty() ||
      module.resultCount() != 0 || module.regions().size() != 1 ||
      module.regions()[0]->blocks().size() != 1) {
    error =
        "builtin.module must have no operands or results and one region with "
        "one block";
    return false;
  }

  const Block& moduleBody = *module.regions()[0]->blocks()[0];
  if (moduleBody.operations().empty()) {
    error = "builtin.module must contain at least one func.func";
    return false;
  }
  std::unordered_set<std::string> symbols;
  for (const auto& functionPtr : moduleBody.operations()) {
    Operation& function = *functionPtr;
    if (function.name() != "func.func" || !function.operands().empty() ||
        function.resultCount() != 0) {
      error = "builtin.module body must contain only func.func operations";
      return false;
    }
    const Attribute* symbolAttribute = function.findAttribute("sym_name");
    const std::string* symbol =
        symbolAttribute ? std::get_if<std::string>(symbolAttribute) : nullptr;
    if (!symbol || symbol->empty()) {
      error = "func.func requires a nonempty sym_name attribute";
      return false;
    }
    if (!symbols.insert(*symbol).second) {
      error = "duplicate function symbol @" + *symbol;
      return false;
    }
    const std::string context = "func.func @" + *symbol + ": ";
    if (function.regions().size() != 1 ||
        function.regions()[0]->blocks().size() != 1) {
      error = context + "expected one region with one block";
      return false;
    }

    const Block& body = *function.regions()[0]->blocks()[0];
    std::unordered_set<const Value*> available;
    for (const auto& argument : body.arguments()) {
      available.insert(argument.get());
    }
    for (std::size_t index = 0; index < body.operations().size(); ++index) {
      const Operation& child = *body.operations()[index];
      for (const Value* operand : child.operands()) {
        if (!available.contains(operand)) {
          error = context + "operation #" + std::to_string(index) +
                  " uses a value outside this block or before its definition";
          return false;
        }
      }
      if (child.name() == "func.return") {
        if (index + 1 != body.operations().size() || !child.regions().empty()) {
          error = context +
                  "func.return must be the final operation and have no regions";
          return false;
        }
        continue;
      }
      std::string childError;
      if (!verifyOperation(child, childError)) {
        error =
            context + "operation #" + std::to_string(index) + ": " + childError;
        return false;
      }
      for (std::size_t result = 0; result < child.resultCount(); ++result) {
        available.insert(&child.result(result));
      }
    }

    std::string functionError;
    if (!FuncOp(function).verifyRegions(functionError)) {
      error = context + functionError;
      return false;
    }
  }
  return true;
}

}  // namespace mini::ir
