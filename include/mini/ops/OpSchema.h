#pragma once

#include <array>
#include <string_view>

namespace mini::ops {

// Human-readable, compile-time operation signatures. Symbolic dimensions
// describe relationships that each operation's verifier enforces.
struct PortSpec {
  std::string_view name;
  std::string_view type;
};

struct AttributeSpec {
  std::string_view name;
  std::string_view type;
};

}  // namespace mini::ops
