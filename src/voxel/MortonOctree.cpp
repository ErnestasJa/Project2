#include "voxel/MortonOctree.h"
#include "voxel/Morton.h"
#include "voxel/MNode.h"
#include <util/Bit.h>

namespace {
inline bool NodeSortPredicate(const vox::MNode &i, const vox::MNode &j) {
  return (i.start < j.start);
}

inline bool NodeEqualsPredicate(const vox::MNode &i, const vox::MNode &j) {
  return (i.start == j.start);
}
}

namespace vox {
void MortonOctree::AddNode(MNode node) {
  auto lb = std::lower_bound(m_nodes.begin(), m_nodes.end(), MNode(node.start));
  m_nodes.insert(lb, core::Move(node));
}

void MortonOctree::AddOrphanNode(MNode node) {
  m_nodes.push_back(core::Move(node));
}

bool MortonOctree::IsSorted() {
  return std::is_sorted(m_nodes.begin(), m_nodes.end(), NodeSortPredicate);
}

void MortonOctree::SortLeafNodes() {
  std::sort(m_nodes.begin(), m_nodes.end(), NodeSortPredicate);
}

void MortonOctree::RemoveDuplicateNodes() {
  elog::LogError("void MortonOctree::RemoveDuplicateNodes() Not implemented");
}

bool MortonOctree::CheckNodeFloat(float x, float y, float z) {
  if (x < 0 || y < 0 || z < 0)
    return false;

  return CheckNode(x, y, z);
}

bool MortonOctree::CheckNode(uint32_t x, uint32_t y, uint32_t z) {
  MNode n(x, y, z, 1);
  return std::binary_search(m_nodes.begin(), m_nodes.end(), n,
                            NodeSortPredicate);
}

#include "voxel/VoxelSide.h"
uint8_t MortonOctree::GetVisibleSides(uint32_t x, uint32_t y, uint32_t z,
                                       core::Vector<MNode>::iterator nodeIt) {
  uint8_t sides = ALL;

  static MNode n;
  n.size = 1;
  n.start = encodeMK(x, y + 1, z);

  if (std::binary_search(nodeIt, m_nodes.end(), n, NodeSortPredicate))
    util::RemoveBit(sides, TOP);

  n.start = encodeMK(x, y, z + 1);
  if (std::binary_search(nodeIt, m_nodes.end(), n, NodeSortPredicate))
    util::RemoveBit(sides, FRONT);

  n.start = encodeMK(x + 1, y, z);
  if (std::binary_search(nodeIt, m_nodes.end(), n, NodeSortPredicate))
    util::RemoveBit(sides, LEFT);

  n.start = encodeMK(x - 1, y, z);
  if (std::binary_search(m_nodes.begin(), nodeIt, n, NodeSortPredicate))
    util::RemoveBit(sides, RIGHT);

  n.start = encodeMK(x, y, z - 1);
  if (std::binary_search(m_nodes.begin(), nodeIt, n, NodeSortPredicate))
    util::RemoveBit(sides, BACK);

  n.start = encodeMK(x, y - 1, z);
  if (std::binary_search(m_nodes.begin(), nodeIt, n, NodeSortPredicate))
    util::RemoveBit(sides, BOTTOM);

  return sides;
}

core::Vector<MNode> &MortonOctree::GetChildNodes() { return m_nodes; }
}