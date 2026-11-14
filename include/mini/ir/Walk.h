#pragma once

#include <functional>

namespace mini::ir {

class Operation;

// Preorder walk: visit root, then each nested Region, Block, and Operation.
void walk(Operation& root,
          const std::function<void(Operation&)>& callback);

}  // namespace mini::ir
