#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;
  const Type full = Type::tensor({18});
  const Type tile = Type::tensor({4});
  Value input(full);
  Value init(full);
  std::string error;

  Operation loop("scf.for", {&init}, {full});
  loop.setAttribute("lower_bound", std::int64_t{0});
  loop.setAttribute("upper_bound", std::int64_t{18});
  loop.setAttribute("step", std::int64_t{4});
  auto& body = loop.addRegion().addBlock();
  Value& offset = body.addArgument(Type::index());
  Value& carried = body.addArgument(full);
  Operation& slice = body.append(std::make_unique<Operation>(
      "tensor.extract_slice", std::vector<Value*>{&input, &offset},
      std::vector<Type>{tile}));
  slice.setAttribute("tile_size", std::int64_t{4});
  Operation& inserted = body.append(std::make_unique<Operation>(
      "tensor.insert_slice",
      std::vector<Value*>{&slice.result(0), &carried, &offset},
      std::vector<Type>{full}));
  inserted.setAttribute("tile_size", std::int64_t{4});
  body.append(std::make_unique<Operation>(
      "scf.yield", std::vector<Value*>{&inserted.result(0)},
      std::vector<Type>{}));
  assert(mini::ir::verifyOperation(slice, error));
  assert(mini::ir::verifyOperation(inserted, error));
  assert(mini::ir::verifyOperation(loop, error));

  Operation badStep("scf.for", {&init}, {full});
  badStep.setAttribute("lower_bound", std::int64_t{0});
  badStep.setAttribute("upper_bound", std::int64_t{18});
  badStep.setAttribute("step", std::int64_t{0});
  // TODO(lesson 7.1): the loop verifier rejects a zero step.
  assert(!mini::ir::verifyOperation(badStep, error));
  assert(!error.empty());

  Operation badExtract("tensor.extract_slice", {&input, &offset}, {tile});
  badExtract.setAttribute("tile_size", std::int64_t{8});
  assert(!mini::ir::verifyOperation(badExtract, error));

  Operation badInsert("tensor.insert_slice",
                      {&slice.result(0), &init, &input}, {full});
  badInsert.setAttribute("tile_size", std::int64_t{4});
  assert(!mini::ir::verifyOperation(badInsert, error));

  Operation badYield("scf.yield", {&offset}, {});
  assert(!mini::ir::verifyOperation(badYield, error));
}
