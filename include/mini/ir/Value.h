#pragma once

#include "mini/ir/Type.h"

#include <cstddef>
#include <span>
#include <vector>

namespace mini::ir {

class Operation;

struct Use {
  Operation* owner;
  std::size_t operandIndex;

  bool operator==(const Use&) const = default;
};

// A Value is owned by its defining Operation, or temporarily by the caller
// for a block argument. The Block class will take over that ownership in 2.2.
class Value {
 public:
  Value(Type type, Operation* definingOp = nullptr,
        std::size_t resultIndex = 0);

  Value(const Value&) = delete;
  Value& operator=(const Value&) = delete;
  Value(Value&&) = delete;
  Value& operator=(Value&&) = delete;

  const Type& type() const { return type_; }
  Operation* definingOp() const { return definingOp_; }
  std::size_t resultIndex() const { return resultIndex_; }
  std::span<const Use> uses() const { return uses_; }
  std::size_t useCount() const { return uses_.size(); }

 private:
  friend class Operation;

  // TODO(lesson 2.1): maintain one Use for each operand position.
  void addUse(Operation* owner, std::size_t operandIndex);
  void removeUse(Operation* owner, std::size_t operandIndex) noexcept;

  Type type_;
  Operation* definingOp_;
  std::size_t resultIndex_;
  std::vector<Use> uses_;
};

}  // namespace mini::ir
