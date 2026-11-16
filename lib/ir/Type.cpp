#include "mini/ir/Type.h"

#include <utility>

namespace mini::ir {

Type::Type(Kind kind, std::vector<std::int64_t> shape)
    : kind_(kind), shape_(std::move(shape)) {}

Type Type::f32() { return Type(Kind::F32, {}); }

Type Type::index() { return Type(Kind::Index, {}); }

Type Type::tensor(std::vector<std::int64_t> shape) {
  return Type(Kind::Tensor, std::move(shape));
}

Type Type::vector(std::vector<std::int64_t> shape) {
  return Type(Kind::Vector, std::move(shape));
}

}  // namespace mini::ir
