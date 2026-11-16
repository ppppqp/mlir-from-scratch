#include "mini/pass/TileGenericPass.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mini/ir/Block.h"
#include "mini/ir/FuncOp.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/MiniGenericOp.h"

namespace mini::pass {
namespace {

void cloneScalarRegion(const ir::Operation& source, ir::Operation& target) {
  const ir::Block& sourceBody = *source.regions()[0]->blocks()[0];
  ir::Block& targetBody = target.addRegion().addBlock();
  std::unordered_map<const ir::Value*, ir::Value*> values;
  for (const auto& argument : sourceBody.arguments()) {
    values.emplace(argument.get(), &targetBody.addArgument(argument->type()));
  }
  for (const auto& child : sourceBody.operations()) {
    std::vector<ir::Value*> operands;
    for (const ir::Value* operand : child->operands()) {
      operands.push_back(values.at(operand));
    }
    std::vector<ir::Type> resultTypes;
    for (std::size_t i = 0; i < child->resultCount(); ++i) {
      resultTypes.push_back(child->result(i).type());
    }
    ir::Operation& cloned = targetBody.append(std::make_unique<ir::Operation>(
        std::string(child->name()), std::move(operands), std::move(resultTypes)));
    for (std::size_t i = 0; i < child->resultCount(); ++i) {
      values.emplace(&child->result(i), &cloned.result(i));
    }
  }
}

}  // namespace

bool TileGenericPass::run(ir::Operation& operation, std::string& error) {
  error.clear();
  if (operation.name() != "func.func") {
    error = "tile-generic expects func.func";
    return false;
  }

  constexpr std::int64_t tileSize = 4;
  const ir::Type tileType = ir::Type::tensor({tileSize});
  ir::Block& block = ir::FuncOp(operation).entryBlock();
  // TODO(lesson 7.2): find a one-dimensional mini.generic in this block.
  // Build tensor.empty and scf.for before it. The loop carries the full
  // output tensor and has (index IV, tensor iter_arg) block arguments.
  // Inside the loop, extract two padded tensor<4xf32> slices using IV and
  // tile_size=4. Clone the original generic scalar region into a new generic
  // over those slices. Insert its result into iter_arg at IV and scf.yield
  // the updated tensor. Verify the new operations before changing uses.
  // Finally RAUW the old generic result with the loop result and erase it.
  // The final tile inserts only min(4, N - IV) valid lanes.
  for (std::size_t index = 0; index < block.operations().size();) {
    ir::Operation& original = *block.operations()[index];
    if (original.name() != ops::MiniGenericOp::name ||
        original.resultCount() != 1 ||
        original.result(0).type().kind() != ir::Type::Kind::Tensor ||
        original.result(0).type().shape().size() != 1) {
      ++index;
      continue;
    }
    std::string verifyError;
    if (!ir::verifyOperation(original, verifyError)) {
      error = "cannot tile invalid mini.generic: " + verifyError;
      return false;
    }

    const ir::Type fullType = original.result(0).type();
    const std::int64_t length = fullType.shape()[0];
    auto empty = std::make_unique<ir::Operation>(
        "tensor.empty", std::vector<ir::Value*>{},
        std::vector<ir::Type>{fullType});
    auto loop = std::make_unique<ir::Operation>(
        "scf.for", std::vector<ir::Value*>{&empty->result(0)},
        std::vector<ir::Type>{fullType});
    loop->setAttribute("lower_bound", std::int64_t{0});
    loop->setAttribute("upper_bound", length);
    loop->setAttribute("step", tileSize);

    ir::Block& loopBody = loop->addRegion().addBlock();
    ir::Value& offset = loopBody.addArgument(ir::Type::index());
    ir::Value& carried = loopBody.addArgument(fullType);
    ir::Operation& aTile = loopBody.append(std::make_unique<ir::Operation>(
        "tensor.extract_slice",
        std::vector<ir::Value*>{original.operands()[0], &offset},
        std::vector<ir::Type>{tileType}));
    aTile.setAttribute("tile_size", tileSize);
    ir::Operation& bTile = loopBody.append(std::make_unique<ir::Operation>(
        "tensor.extract_slice",
        std::vector<ir::Value*>{original.operands()[1], &offset},
        std::vector<ir::Type>{tileType}));
    bTile.setAttribute("tile_size", tileSize);
    ir::Operation& tiledGeneric = loopBody.append(std::make_unique<ir::Operation>(
        "mini.generic",
        std::vector<ir::Value*>{&aTile.result(0), &bTile.result(0)},
        std::vector<ir::Type>{tileType}));
    cloneScalarRegion(original, tiledGeneric);
    ir::Operation& inserted = loopBody.append(std::make_unique<ir::Operation>(
        "tensor.insert_slice",
        std::vector<ir::Value*>{&tiledGeneric.result(0), &carried, &offset},
        std::vector<ir::Type>{fullType}));
    inserted.setAttribute("tile_size", tileSize);
    loopBody.append(std::make_unique<ir::Operation>(
        "scf.yield", std::vector<ir::Value*>{&inserted.result(0)},
        std::vector<ir::Type>{}));

    if (!ir::verifyOperation(*empty, verifyError) ||
        !ir::verifyOperation(*loop, verifyError)) {
      error = "constructed invalid tiled IR: " + verifyError;
      return false;
    }

    block.insertBefore(original, std::move(empty));
    ir::Operation& insertedLoop = block.insertBefore(original, std::move(loop));
    original.result(0).replaceAllUsesWith(insertedLoop.result(0));
    block.erase(original);
    index += 2;
  }
  return true;
}

}  // namespace mini::pass
