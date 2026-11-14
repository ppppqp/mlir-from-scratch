#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "mini/ir/Value.h"
#include "mini/ir/Attribute.h"

namespace mini::ir {

class Region;

// The owning Block (added in 2.2) will hold unique_ptr<Operation>. Raw
// operand pointers are non-owning; each result is owned by this Operation.
class Operation {
 public:
  Operation(std::string name, std::vector<Value*> operands,
            std::vector<Type> resultTypes);
  ~Operation();

  Operation(const Operation&) = delete;
  Operation& operator=(const Operation&) = delete;
  Operation(Operation&&) = delete;
  Operation& operator=(Operation&&) = delete;

  std::string_view name() const { return name_; }
  std::span<Value* const> operands() const { return operands_; }
  std::size_t resultCount() const { return results_.size(); }
  Value& result(std::size_t index) { return *results_.at(index); }
  const Value& result(std::size_t index) const { return *results_.at(index); }
  std::span<const std::unique_ptr<Region>> regions() const { return regions_; }

  // TODO(lesson 3.2): store typed metadata such as sym_name and function_type.
  void setAttribute(std::string key, Attribute value);
  const Attribute* findAttribute(std::string_view key) const;

  // TODO(lesson 2.2): an Operation can own nested Regions.
  Region& addRegion();

  // TODO(lesson 2.1): update both the old and new Value use lists.
  void setOperand(std::size_t index, Value* replacement);

 private:
  std::string name_;
  std::vector<Value*> operands_;
  std::vector<std::unique_ptr<Value>> results_;
  std::vector<std::unique_ptr<Region>> regions_;
  std::map<std::string, Attribute, std::less<>> attributes_;
};

}  // namespace mini::ir
