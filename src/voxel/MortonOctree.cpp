#include "voxel/MortonOctree.h"
#include "voxel/Morton.h"
#include "voxel/VoxNode.h"
#include <util/Bit.h>

namespace {
inline bool NodeSortPredicate(const vox::VoxNode &i, const vox::VoxNode &j) {
  return (i.start < j.start);
}

inline bool ActiveNodeSortPredicate(const vox::VoxNode &i, const vox::VoxNode &j) {
  return (i.size > 0 && j.size > 0 && i.start < j.start);
}

inline bool NodeEqualsPredicate(const vox::VoxNode &i, const vox::VoxNode &j) {
  return (i.start == j.start);
}
}

namespace vox {
void MortonOctree::AddNode(VoxNode node) {
  auto lb = std::lower_bound(m_nodes.begin(), m_nodes.end(), VoxNode(node.start));

  if(lb != m_nodes.end()){
    if(lb->size == -1) {
      lb->Assign(node);
    }
    else{
      m_nodes.insert(lb, core::Move(node));
    }
  }
  else {
    m_nodes.emplace_back(node);
  }
}

void MortonOctree::Remove(VoxNode node) {
  uint32_t x, y, z;
  vox::decodeMK(node.start, x,y,z);
  RemoveNode(x,y,z);
}

void MortonOctree::AddOrphanNode(VoxNode node) {
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
  VoxNode n(x, y, z, 1);
  auto node = std::lower_bound(m_nodes.begin(), m_nodes.end(), n,
                               NodeSortPredicate);

  return node != m_nodes.end() && node->start == n.start && node->size > 0;
}

#include "voxel/VoxelSide.h"
uint8_t MortonOctree::GetVisibleSides(uint32_t x, uint32_t y, uint32_t z,
                                       core::Vector<VoxNode>::iterator nodeIt) {
  uint8_t sides = ALL;

  static VoxNode n;
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

core::Vector<VoxNode> &MortonOctree::GetNodes() { return m_nodes; }

bool MortonOctree::RemoveNode(uint32_t x, uint32_t y, uint32_t z) {
  auto start = vox::encodeMK(x,y,z);

  auto lb = std::lower_bound(m_nodes.begin(), m_nodes.end(),
                             VoxNode(start), NodeSortPredicate);

  if(lb != m_nodes.end() && lb->start == start)
  {
    uint32_t nx, ny, nz;
    vox::decodeMK(lb->start, nx, ny, nz);
    elog::LogInfo(core::string::format("Removed node at: [{}, {}, {}]", nx, ny, nz));
    lb->size = -1;
    return true;
  }

  return false;
}
}