#include "mini/ops/TilingOps.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <variant>

#include "mini/ir/Block.h"
#include "mini/ir/Operation.h"
#include "mini/ir/Region.h"
#include "mini/ir/Verify.h"
#include "mini/ops/MiniGenericOp.h"

namespace mini::ops {
namespace {

const std::int64_t* integerAttribute(const ir::Operation& operation,
                                     std::string_view key) {
  const ir::Attribute* attribute = operation.findAttribute(key);
  return attribute ? std::get_if<std::int64_t>(attribute) : nullptr;
}

bool positiveRankOneTensor(const ir::Type& type) {
  return type.kind() == ir::Type::Kind::Tensor && type.shape().size() == 1 &&
         type.shape()[0] > 0;
}

}  // namespace

ScfForOp::ScfForOp(const ir::Operation& operation) : operation_(&operation) {
  if (operation.name() != name) throw std::invalid_argument("expected scf.for");
}

bool ScfForOp::verify(std::string& error) const {
  error.clear();
  // TODO(lesson 7.1): require lower_bound=0, upper_bound>0, step>0;
  // one init tensor operand and matching result; one region, one block;
  // body args (index IV, tensor iter_arg); final scf.yield of matching tensor.
  // Verify body-local SSA order and nested operations. The loop may capture
  // values defined before it in the enclosing block (e.g. input tensors).
  const auto* lower = integerAttribute(*operation_, "lower_bound");
  const auto* upper = integerAttribute(*operation_, "upper_bound");
  const auto* step = integerAttribute(*operation_, "step");
  if (!lower || !upper || !step || *lower != 0 || *upper <= 0 || *step <= 0) {
    error = "scf.for requires lower_bound=0, upper_bound>0, and step>0";
    return false;
  }
  if (operation_->operands().size() != 1 || operation_->resultCount() != 1 ||
      !positiveRankOneTensor(operation_->operands()[0]->type()) ||
      operation_->result(0).type() != operation_->operands()[0]->type()) {
    error = "scf.for requires one rank-1 tensor init and matching result";
    return false;
  }
  if (operation_->regions().size() != 1 ||
      operation_->regions()[0]->blocks().size() != 1) {
    error = "scf.for requires one region with one block";
    return false;
  }
  const ir::Block& body = *operation_->regions()[0]->blocks()[0];
  const auto arguments = body.arguments();
  if (arguments.size() != 2 || arguments[0]->type() != ir::Type::index() ||
      arguments[1]->type() != operation_->result(0).type()) {
    error = "scf.for body requires index IV and matching tensor iter_arg";
    return false;
  }
  const auto operations = body.operations();
  if (operations.empty() || operations.back()->name() != ScfYieldOp::name) {
    error = "scf.for body must end with scf.yield";
    return false;
  }

  std::unordered_set<const ir::Value*> localResults;
  for (const auto& child : operations) {
    for (std::size_t i = 0; i < child->resultCount(); ++i) {
      localResults.insert(&child->result(i));
    }
  }
  std::unordered_set<const ir::Value*> available{arguments[0].get(),
                                                 arguments[1].get()};
  for (std::size_t index = 0; index < operations.size(); ++index) {
    const ir::Operation& child = *operations[index];
    if (child.name() != TensorExtractSliceOp::name &&
        child.name() != TensorInsertSliceOp::name &&
        child.name() != MiniGenericOp::name &&
        child.name() != ScfYieldOp::name) {
      error = "scf.for body contains unsupported operation";
      return false;
    }
    if (child.name() == ScfYieldOp::name && index + 1 != operations.size()) {
      error = "scf.yield must be the final operation";
      return false;
    }
    for (const ir::Value* operand : child.operands()) {
      if (localResults.contains(operand) && !available.contains(operand)) {
        error = "scf.for body uses a local result before its definition";
        return false;
      }
    }
    std::string childError;
    if (!ir::verifyOperation(child, childError)) {
      error = "scf.for body operation #" + std::to_string(index) + ": " +
              childError;
      return false;
    }
    if (child.name() == ScfYieldOp::name &&
        child.operands()[0]->type() != operation_->result(0).type()) {
      error = "scf.yield type must match scf.for result";
      return false;
    }
    for (std::size_t i = 0; i < child.resultCount(); ++i) {
      available.insert(&child.result(i));
    }
  }
  return true;
}

ScfYieldOp::ScfYieldOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name)
    throw std::invalid_argument("expected scf.yield");
}

bool ScfYieldOp::verify(std::string& error) const {
  error.clear();
  // TODO(lesson 7.1): one tensor operand, no results, no regions.
  if (operation_->operands().size() != 1 || operation_->resultCount() != 0 ||
      !operation_->regions().empty() ||
      !positiveRankOneTensor(operation_->operands()[0]->type())) {
    error =
        "scf.yield requires one rank-1 tensor operand and no results or "
        "regions";
    return false;
  }
  return true;
}

TensorExtractSliceOp::TensorExtractSliceOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name)
    throw std::invalid_argument("expected tensor.extract_slice");
}

bool TensorExtractSliceOp::verify(std::string& error) const {
  error.clear();
  // TODO(lesson 7.1): source tensor and index offset operands; positive
  // tile_size int64 attribute; tensor<tile_size x f32> result; no regions.
  const auto* tileSize = integerAttribute(*operation_, "tile_size");
  if (operation_->operands().size() != 2 || operation_->resultCount() != 1 ||
      !operation_->regions().empty() || !tileSize || *tileSize <= 0 ||
      !positiveRankOneTensor(operation_->operands()[0]->type()) ||
      operation_->operands()[1]->type() != ir::Type::index() ||
      !positiveRankOneTensor(operation_->result(0).type()) ||
      operation_->result(0).type().shape()[0] != *tileSize) {
    error =
        "tensor.extract_slice requires tensor source, index offset, and "
        "matching positive tile_size";
    return false;
  }
  return true;
}

TensorInsertSliceOp::TensorInsertSliceOp(const ir::Operation& operation)
    : operation_(&operation) {
  if (operation.name() != name)
    throw std::invalid_argument("expected tensor.insert_slice");
}

bool TensorInsertSliceOp::verify(std::string& error) const {
  error.clear();
  // TODO(lesson 7.1): tile tensor, destination tensor, and index offset
  // operands; result type equals destination; positive tile_size int64
  // attribute equals tile length; no regions.
  const auto* tileSize = integerAttribute(*operation_, "tile_size");
  if (operation_->operands().size() != 3 || operation_->resultCount() != 1 ||
      !operation_->regions().empty() || !tileSize || *tileSize <= 0 ||
      !positiveRankOneTensor(operation_->operands()[0]->type()) ||
      operation_->operands()[0]->type().shape()[0] != *tileSize ||
      !positiveRankOneTensor(operation_->operands()[1]->type()) ||
      operation_->operands()[2]->type() != ir::Type::index() ||
      operation_->result(0).type() != operation_->operands()[1]->type()) {
    error =
        "tensor.insert_slice requires tile, destination, index offset, and "
        "matching positive tile_size";
    return false;
  }
  return true;
}

}  // namespace mini::ops
