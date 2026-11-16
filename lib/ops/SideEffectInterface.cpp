#include "mini/ops/SideEffectInterface.h"

#include <array>
#include <string_view>

#include "mini/ir/Operation.h"
#include "mini/ops/MiniAddOp.h"
#include "mini/ops/MiniDpsAddOp.h"
#include "mini/ops/MiniDpsReluOp.h"
#include "mini/ops/MiniMatmulOp.h"
#include "mini/ops/MiniReluOp.h"
#include "mini/ops/TensorEmptyOp.h"

namespace mini::ops {
namespace {

struct Entry {
  std::string_view name;
  bool hasSideEffects;
};

constexpr std::array entries{
    Entry{MiniAddOp::name, MiniAddOp::hasSideEffects()},
    Entry{MiniReluOp::name, MiniReluOp::hasSideEffects()},
    Entry{MiniMatmulOp::name, MiniMatmulOp::hasSideEffects()},
    Entry{MiniDpsAddOp::name, MiniDpsAddOp::hasSideEffects()},
    Entry{MiniDpsReluOp::name, MiniDpsReluOp::hasSideEffects()},
    Entry{TensorEmptyOp::name, TensorEmptyOp::hasSideEffects()},
};

}  // namespace

bool SideEffectInterface::hasSideEffects(const ir::Operation& operation) {
  for (const Entry& entry : entries) {
    if (operation.name() == entry.name) {
      return entry.hasSideEffects;
    }
  }
  return true;
}

}  // namespace mini::ops
