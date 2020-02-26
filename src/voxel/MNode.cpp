#include "voxel/MNode.h"
#include "voxel/Morton.h"

namespace vox {
MNode::MNode(MNode &&n) noexcept
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

MNode::MNode(uint32_t x, uint32_t y, uint32_t z, uint8_t nodeSize) {
  start = encodeMK(x, y, z);
  size = nodeSize;
  r = g = b = 255;
}

MNode::MNode(core::pod::Vec3<uint32_t> pos, core::pod::Vec3<uint32_t> color, uint8_t nodeSize){
  start = encodeMK(pos.x, pos.y, pos.z);
  r = color.r;
  g = color.g;
  b = color.b;
  size = nodeSize;
}

MNode::MNode(uint32_t morton, uint8_t nodeSize, uint8_t red, uint8_t green,
             uint8_t blue) {
  start = morton;
  size = nodeSize;
  r = red;
  g = green;
  b = blue;
}

MNode::MNode(uint32_t morton, uint8_t nodeSize) {
  start = morton;
  size = nodeSize;
  r = g = b = 255;
}

MNode::MNode() {
  start = 0;
  size = 0;
  r = g = b = 255;
}

bool MNode::operator<(const MNode &other)
    const /// ordering: z order, plus on equal sizes largest first
{
  return start == other.start ? size > other.size : start < other.start;
}

bool MNode::operator==(const MNode &other) const {
  return start == other.start && size == other.size;
}
}