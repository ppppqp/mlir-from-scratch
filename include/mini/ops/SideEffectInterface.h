#pragma once

namespace mini::ir {
class Operation;
}

namespace mini::ops {

// A small op interface queried by transformations. Unknown operations are
// conservatively treated as having side effects.
class SideEffectInterface {
 public:
  static bool hasSideEffects(const ir::Operation& operation);
};

}  // namespace mini::ops
