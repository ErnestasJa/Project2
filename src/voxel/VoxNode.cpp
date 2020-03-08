#include "voxel/VoxNode.h"
#include "voxel/Morton.h"

namespace vox {
VoxNode::VoxNode(VoxNode &&n) noexcept
    : start(n.start), size(n.size), r(n.r), g(n.g), b(n.b) {}

/*MNode& MNode::operator=(MNode&& x) noexcept //fixme
{
    start = x.start;
    r = x.r;
    g = x.g;
    b = x.b;
    size  = x.size;
    return *this;
}*/

VoxNode::VoxNode(uint32_t x, uint32_t y, uint32_t z, uint8_t nodeSize) {
  start = encodeMK(x, y, z);
  size = nodeSize;
  r = g = b = 255;
}

VoxNode::VoxNode(core::pod::Vec3<uint32_t> pos, core::pod::Vec3<uint8_t> color, uint8_t nodeSize){
  start = encodeMK(pos.x, pos.y, pos.z);
  r = color.r;
  g = color.g;
  b = color.b;
  size = nodeSize;
}

VoxNode::VoxNode(uint32_t morton, uint8_t nodeSize, uint8_t red, uint8_t green,
             uint8_t blue) {
  start = morton;
  size = nodeSize;
  r = red;
  g = green;
  b = blue;
}

VoxNode::VoxNode(uint32_t morton, uint8_t nodeSize) {
  start = morton;
  size = nodeSize;
  r = g = b = 255;
}

VoxNode::VoxNode() {
  start = 0;
  size = 0;
  r = g = b = 255;
}

bool VoxNode::operator<(const VoxNode &other)
    const /// ordering: z order, plus on equal sizes largest first
{
  return start == other.start ? size > other.size : start < other.start;
}

bool VoxNode::operator==(const VoxNode &other) const {
  return start == other.start && size == other.size;
}

void VoxNode::Assign(const VoxNode &node) {
  size = node.size;
  start = node.start;
  r = node.r;
  g = node.g;
  b = node.b;
}
}