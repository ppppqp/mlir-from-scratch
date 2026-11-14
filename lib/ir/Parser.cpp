#include "mini/ir/Parser.h"

#include "mini/ir/Attribute.h"
#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"

#include <charconv>
#include <cctype>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mini::ir {
namespace {

bool isNameChar(char ch) {
  const auto byte = static_cast<unsigned char>(ch);
  return std::isalnum(byte) || ch == '_' || ch == '.' || ch == '$' || ch == '-';
}

class Reader {
 public:
  explicit Reader(std::string_view source) : source_(source) {}

  std::unique_ptr<Operation> parse() {
    expectWord("module");
    expect('{');
    auto module = std::make_unique<Operation>("builtin.module",
                                              std::vector<Value*>{},
                                              std::vector<Type>{});
    Block& body = module->addRegion().addBlock();
    std::unordered_set<std::string> functionNames;
    while (!consume('}')) {
      parseFunction(body, functionNames);
    }
    if (functionNames.empty()) {
      fail("module must contain at least one function");
    }
    skipTrivia();
    if (position_ != source_.size()) {
      fail("unexpected text after module");
    }
    return module;
  }

 private:
  [[noreturn]] void fail(std::string_view message) const {
    throw std::runtime_error("parse error at offset " +
                             std::to_string(position_) + ": " +
                             std::string(message));
  }

  void skipTrivia() {
    while (position_ < source_.size()) {
      const auto ch = static_cast<unsigned char>(source_[position_]);
      if (std::isspace(ch)) {
        ++position_;
        continue;
      }
      if (source_.substr(position_, 2) == "//") {
        position_ += 2;
        while (position_ < source_.size() && source_[position_] != '\n') {
          ++position_;
        }
        continue;
      }
      break;
    }
  }

  char peek() {
    skipTrivia();
    return position_ < source_.size() ? source_[position_] : '\0';
  }

  bool consume(char expected) {
    if (peek() != expected) {
      return false;
    }
    ++position_;
    return true;
  }

  void expect(char expected) {
    if (!consume(expected)) {
      fail(std::string("expected '") + expected + "'");
    }
  }

  bool consumeWord(std::string_view word) {
    skipTrivia();
    if (source_.substr(position_, word.size()) != word) {
      return false;
    }
    const std::size_t end = position_ + word.size();
    if (end < source_.size() && isNameChar(source_[end])) {
      return false;
    }
    position_ = end;
    return true;
  }

  void expectWord(std::string_view word) {
    if (!consumeWord(word)) {
      fail("expected '" + std::string(word) + "'");
    }
  }

  void expectArrow() {
    skipTrivia();
    if (source_.substr(position_, 2) != "->") {
      fail("expected '->'");
    }
    position_ += 2;
  }

  std::string parseName(char prefix) {
    expect(prefix);
    const std::size_t begin = position_;
    while (position_ < source_.size() && isNameChar(source_[position_])) {
      ++position_;
    }
    if (begin == position_) {
      fail("expected name after sigil");
    }
    return std::string(source_.substr(begin, position_ - begin));
  }

  std::string parseQuotedOperationName() {
    expect('"');
    const std::size_t begin = position_;
    while (position_ < source_.size() && source_[position_] != '"') {
      if (source_[position_] == '\\' || source_[position_] == '\n') {
        fail("escaped or multiline operation names are unsupported");
      }
      ++position_;
    }
    if (position_ == source_.size()) {
      fail("unterminated operation name");
    }
    const std::string name(source_.substr(begin, position_ - begin));
    ++position_;
    return name;
  }

  std::int64_t parseDimension() {
    skipTrivia();
    const std::size_t begin = position_;
    while (position_ < source_.size() &&
           std::isdigit(static_cast<unsigned char>(source_[position_]))) {
      ++position_;
    }
    if (begin == position_) {
      fail("expected static dimension");
    }
    std::int64_t dimension = 0;
    const char* first = source_.data() + begin;
    const char* last = source_.data() + position_;
    if (std::from_chars(first, last, dimension).ec != std::errc{} ||
        dimension <= 0) {
      fail("dimension must be a positive 64-bit integer");
    }
    return dimension;
  }

  Type parseType() {
    if (consumeWord("f32")) {
      return Type::f32();
    }
    expectWord("tensor");
    expect('<');
    const std::int64_t first = parseDimension();
    expect('x');
    if (std::isdigit(static_cast<unsigned char>(peek()))) {
      const std::int64_t second = parseDimension();
      expect('x');
      expectWord("f32");
      expect('>');
      return Type::tensor({first, second});
    }
    expectWord("f32");
    expect('>');
    return Type::tensor({first});
  }

  void parseComputation(Block& body,
                        std::unordered_map<std::string, Value*>& values) {
    const std::string resultName = parseName('%');
    if (values.contains(resultName)) {
      fail("duplicate SSA name");
    }
    expect('=');
    const std::string operationName = parseQuotedOperationName();
    expect('(');
    std::vector<Value*> operands;
    if (!consume(')')) {
      do {
        const std::string name = parseName('%');
        const auto it = values.find(name);
        if (it == values.end()) {
          fail("use of undefined SSA value");
        }
        operands.push_back(it->second);
      } while (consume(','));
      expect(')');
    }
    expect(':');
    expect('(');
    std::vector<Type> declaredOperandTypes;
    if (!consume(')')) {
      do {
        declaredOperandTypes.push_back(parseType());
      } while (consume(','));
      expect(')');
    }
    expectArrow();
    const Type resultType = parseType();
    if (declaredOperandTypes.size() != operands.size()) {
      fail("operand type count does not match operand count");
    }
    for (std::size_t i = 0; i < operands.size(); ++i) {
      if (declaredOperandTypes[i] != operands[i]->type()) {
        fail("declared operand type does not match SSA value type");
      }
    }
    auto operation = std::make_unique<Operation>(
        operationName, std::move(operands), std::vector<Type>{resultType});
    std::string error;
    if (!verifyOperation(*operation, error)) {
      fail(error);
    }
    Value* result = &operation->result(0);
    body.append(std::move(operation));
    values.emplace(resultName, result);
  }

  void parseReturn(Block& body,
                   const std::unordered_map<std::string, Value*>& values,
                   const Type& expectedType) {
    const std::string name = parseName('%');
    expect(':');
    const Type declaredType = parseType();
    const auto it = values.find(name);
    if (it == values.end()) {
      fail("return of undefined SSA value");
    }
    if (declaredType != it->second->type() || declaredType != expectedType) {
      fail("return type does not match function result type");
    }
    body.append(std::make_unique<Operation>(
        "func.return", std::vector<Value*>{it->second}, std::vector<Type>{}));
  }

  void parseFunction(Block& moduleBody,
                     std::unordered_set<std::string>& functionNames) {
    expectWord("func.func");
    const std::string functionName = parseName('@');
    if (!functionNames.insert(functionName).second) {
      fail("duplicate function name");
    }
    expect('(');
    std::vector<std::pair<std::string, Type>> arguments;
    if (!consume(')')) {
      do {
        std::string name = parseName('%');
        expect(':');
        arguments.emplace_back(std::move(name), parseType());
      } while (consume(','));
      expect(')');
    }
    expectArrow();
    const Type resultType = parseType();
    expect('{');

    auto function = std::make_unique<Operation>("func.func",
                                                std::vector<Value*>{},
                                                std::vector<Type>{});
    function->setAttribute("sym_name", functionName);
    FunctionType signature;
    for (const auto& argument : arguments) {
      signature.inputs.push_back(argument.second);
    }
    signature.results.push_back(resultType);
    function->setAttribute("function_type", std::move(signature));
    Block& body = function->addRegion().addBlock();
    std::unordered_map<std::string, Value*> values;
    for (const auto& [name, type] : arguments) {
      if (values.contains(name)) {
        fail("duplicate function argument name");
      }
      values.emplace(name, &body.addArgument(type));
    }

    while (true) {
      if (peek() == '%') {
        parseComputation(body, values);
        continue;
      }
      if (consumeWord("func.return") || consumeWord("return")) {
        parseReturn(body, values, resultType);
        expect('}');
        break;
      }
      fail("expected computation or func.return");
    }
    moduleBody.append(std::move(function));
  }

  std::string_view source_;
  std::size_t position_ = 0;
};

}  // namespace

std::unique_ptr<Operation> parseModule(std::string_view source) {
  return Reader(source).parse();
}

}  // namespace mini::ir
