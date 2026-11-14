#include "mini/ir/Printer.h"

#include "mini/ir/Attribute.h"
#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace mini::ir {
namespace {

const Block& onlyBlock(const Operation& operation) {
  if (operation.regions().size() != 1 ||
      operation.regions()[0]->blocks().size() != 1) {
    throw std::invalid_argument("expected exactly one region with one block");
  }
  return *operation.regions()[0]->blocks()[0];
}

template <typename T>
const T& requiredAttribute(const Operation& operation, std::string_view key) {
  const Attribute* attribute = operation.findAttribute(key);
  const T* value = attribute ? std::get_if<T>(attribute) : nullptr;
  if (!value) {
    throw std::invalid_argument("missing or invalid attribute: " +
                                std::string(key));
  }
  return *value;
}

std::string printType(const Type& type) {
  if (type.kind() == Type::Kind::F32) {
    return "f32";
  }
  const auto shape = type.shape();
  if (shape.empty() || shape.size() > 2) {
    throw std::invalid_argument("unsupported tensor rank");
  }
  std::string result = "tensor<";
  for (const auto dimension : shape) {
    if (dimension <= 0) {
      throw std::invalid_argument("tensor dimensions must be positive");
    }
    result += std::to_string(dimension) + "x";
  }
  return result + "f32>";
}

const std::string& nameOf(
    const Value* value,
    const std::unordered_map<const Value*, std::string>& names) {
  const auto it = names.find(value);
  if (it == names.end()) {
    throw std::invalid_argument("operand is not defined earlier in function");
  }
  return it->second;
}

void printFunction(std::ostringstream& output, const Operation& function) {
  if (function.name() != "func.func" || function.resultCount() != 0 ||
      !function.operands().empty()) {
    throw std::invalid_argument("expected func.func without SSA results");
  }
  const auto& symbol = requiredAttribute<std::string>(function, "sym_name");
  const auto& signature =
      requiredAttribute<FunctionType>(function, "function_type");
  const Block& body = onlyBlock(function);
  if (signature.results.size() != 1 ||
      signature.inputs.size() != body.arguments().size()) {
    throw std::invalid_argument("function signature does not match its block");
  }

  output << "  func.func @" << symbol << '(';
  std::unordered_map<const Value*, std::string> names;
  for (std::size_t i = 0; i < body.arguments().size(); ++i) {
    if (body.arguments()[i]->type() != signature.inputs[i]) {
      throw std::invalid_argument("function argument type does not match signature");
    }
    if (i) output << ", ";
    const std::string name = "%arg" + std::to_string(i);
    names.emplace(body.arguments()[i].get(), name);
    output << name << ": " << printType(signature.inputs[i]);
  }
  output << ") -> " << printType(signature.results[0]) << " {\n";

  std::size_t resultIndex = 0;
  bool sawReturn = false;
  for (const auto& operation : body.operations()) {
    if (sawReturn) {
      throw std::invalid_argument("operation after func.return");
    }
    if (operation->name() == "func.return") {
      if (operation->operands().size() != 1 || operation->resultCount() != 0 ||
          operation->operands()[0]->type() != signature.results[0]) {
        throw std::invalid_argument("invalid func.return");
      }
      output << "    func.return " << nameOf(operation->operands()[0], names)
             << " : " << printType(signature.results[0]) << "\n";
      sawReturn = true;
      continue;
    }
    std::string error;
    if (!verifyOperation(*operation, error)) {
      throw std::invalid_argument(error);
    }
    const std::string resultName = "%" + std::to_string(resultIndex++);
    output << "    " << resultName << " = \"" << operation->name() << "\"(";
    for (std::size_t i = 0; i < operation->operands().size(); ++i) {
      if (i) output << ", ";
      output << nameOf(operation->operands()[i], names);
    }
    output << ") : (";
    for (std::size_t i = 0; i < operation->operands().size(); ++i) {
      if (i) output << ", ";
      output << printType(operation->operands()[i]->type());
    }
    output << ") -> " << printType(operation->result(0).type()) << "\n";
    names.emplace(&operation->result(0), resultName);
  }
  if (!sawReturn) {
    throw std::invalid_argument("function body has no func.return");
  }
  output << "  }\n";
}

}  // namespace

std::string printModule(const Operation& module) {
  if (module.name() != "builtin.module" || module.resultCount() != 0 ||
      !module.operands().empty()) {
    throw std::invalid_argument("expected builtin.module without SSA results");
  }
  const Block& body = onlyBlock(module);
  if (body.operations().empty()) {
    throw std::invalid_argument("module must contain at least one function");
  }
  std::ostringstream output;
  output << "module {\n";
  for (const auto& function : body.operations()) {
    printFunction(output, *function);
  }
  output << "}\n";
  return output.str();
}

}  // namespace mini::ir
