#pragma once

#include <memory>
#include <span>
#include <vector>

#include "mini/ir/Operation.h"

namespace mini::ir {

// A Block owns its arguments and its operations. Operations are appended in
// SSA definition order. On destruction, users must be removed before defs.
class Block {
 public:
  Block() = default;
  ~Block();

  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;
  Block(Block&&) = delete;
  Block& operator=(Block&&) = delete;

  Value& addArgument(Type type);
  Operation& append(std::unique_ptr<Operation> operation);
  Operation& insertBefore(Operation& anchor,
                          std::unique_ptr<Operation> operation);
  void erase(Operation& operation);

  std::span<const std::unique_ptr<Value>> arguments() const {
    return arguments_;
  }
  std::span<const std::unique_ptr<Operation>> operations() const {
    return operations_;
  }

 private:
  std::vector<std::unique_ptr<Value>> arguments_;
  std::vector<std::unique_ptr<Operation>> operations_;
};

}  // namespace mini::ir
