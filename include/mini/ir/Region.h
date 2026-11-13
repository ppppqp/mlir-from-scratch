#pragma once

#include <memory>
#include <span>
#include <vector>

namespace mini::ir {

class Block;

// A Region owns Blocks. This course initially uses one Block per Region.
class Region {
 public:
  Region() = default;
  ~Region();

  Region(const Region&) = delete;
  Region& operator=(const Region&) = delete;
  Region(Region&&) = delete;
  Region& operator=(Region&&) = delete;

  Block& addBlock();
  std::span<const std::unique_ptr<Block>> blocks() const { return blocks_; }

 private:
  std::vector<std::unique_ptr<Block>> blocks_;
};

}  // namespace mini::ir
