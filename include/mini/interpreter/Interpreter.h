#pragma once

#include "mini/ir/Type.h"

#include <span>
#include <vector>

namespace mini::ir {
class Operation;
}

namespace mini::interpreter {

// Row-major f32 tensor data paired with its IR type.
struct TensorData {
  ir::Type type;
  std::vector<float> elements;
};

// Execute one func.func with a single entry block and one tensor result.
// Supports mini.add, mini.relu, mini.matmul, mini.dps_add, and mini.dps_relu.
TensorData runFunction(const ir::Operation& function,
                       std::span<const TensorData> inputs);

}  // namespace mini::interpreter
