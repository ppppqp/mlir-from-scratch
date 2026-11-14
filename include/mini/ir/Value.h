#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include "mini/ir/Type.h"

namespace mini::ir {

class Operation;
class Block;

struct Use {
  Operation* owner;
  std::size_t operandIndex;

  bool operator==(const Use&) const = default;
};

// Operation results are owned by their defining Operation. Block arguments
// are owned by their Block. Standalone Values remain useful in small tests.
class Value {
 public:
  Value(Type type, Operation* definingOp = nullptr,
        std::size_t resultIndex = 0);
  Value(Type type, Block* definingBlock, std::size_t argumentIndex);

  Value(const Value&) = delete;
  Value& operator=(const Value&) = delete;
  Value(Value&&) = delete;
  Value& operator=(Value&&) = delete;

  const Type& type() const { return type_; }
  Operation* definingOp() const { return definingOp_; }
  std::size_t resultIndex() const { return resultIndex_; }
  bool isBlockArgument() const { return definingBlock_ != nullptr; }
  Block* definingBlock() const { return definingBlock_; }
  std::size_t argumentIndex() const { return argumentIndex_; }
  std::span<const Use> uses() const { return uses_; }
  std::size_t useCount() const { return uses_.size(); }

  // TODO(lesson 2.3): replace every operand that uses this Value.
  void replaceAllUsesWith(Value& replacement);

 private:
  friend class Operation;

  // TODO(lesson 2.1): maintain one Use for each operand position.
  void addUse(Operation* owner, std::size_t operandIndex);
  void removeUse(Operation* owner, std::size_t operandIndex) noexcept;

  Type type_;
  Operation* definingOp_;
  std::size_t resultIndex_;
  Block* definingBlock_;
  std::size_t argumentIndex_;
  std::vector<Use> uses_;
};

}  // namespace mini::ir
