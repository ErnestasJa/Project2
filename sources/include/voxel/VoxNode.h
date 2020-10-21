#ifndef MNODE_H
#define MNODE_H

#include "MNodeUtil.h"
#include "core/POD.h"

namespace vox {
struct VoxNode {
public:
  uint32_t start;
  uint32_t size;
  uint8_t r, g, b;

  VoxNode(const VoxNode &node) = default;
  VoxNode(VoxNode &&n) noexcept;
  VoxNode(core::pod::Vec3<uint32_t> pos, core::pod::Vec3<uint8_t> color, uint32_t nodeSize = 1);
  VoxNode(uint32_t x, uint32_t y, uint32_t z, uint32_t nodeSize = 1);
  VoxNode(uint32_t morton, uint32_t nodeSize, uint8_t red, uint8_t green,
        uint8_t blue);
  explicit VoxNode(uint32_t morton, uint32_t nodeSize = 1);
  VoxNode();

  void Assign(const VoxNode& node);

  VoxNode &operator=(VoxNode &&x) noexcept = default;
  bool operator<(const VoxNode &other) const;
  bool operator==(const VoxNode &other) const;
};
}
#endif
