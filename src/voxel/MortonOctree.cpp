#include "voxel/MortonOctree.h"
#include <utils/Bit.h>

namespace {
inline bool NodeSortPredicate(const MNode &i, const MNode &j) {
  return (i.start < j.start);
}

inline bool NodeEqualsPredicate(const MNode &i, const MNode &j) {
  return (i.start == j.start);
}
}

void MortonOctTree::AddNode(MNode node)
{
  auto lb = std::lower_bound(m_nodes.begin(), m_nodes.end(), MNode(node.start));
  m_nodes.insert(lb, core::Move(node));
}

void MortonOctTree::AddOrphanNode(MNode node)
{
    m_nodes.push_back(core::Move(node));
}

bool MortonOctTree::IsSorted()
{
    return std::is_sorted(m_nodes.begin(), m_nodes.end(), NodeSortPredicate);
}

void MortonOctTree::SortLeafNodes()
{
  std::sort(m_nodes.begin(), m_nodes.end(), NodeSortPredicate);
}

void MortonOctTree::RemoveDuplicateNodes()
{
  elog::LogError("void MortonOctTree::RemoveDuplicateNodes() Not implemented");
}

bool MortonOctTree::CheckNodeFloat(float x, float y, float z)
{
    if(x<0||y<0||z<0)
        return false;

    return CheckNode(x,y,z);
}

bool MortonOctTree::CheckNode(uint32_t x, uint32_t y, uint32_t z)
{
    MNode n(x,y,z,1);
    return std::binary_search(m_nodes.begin(), m_nodes.end(), n,
        NodeSortPredicate);
}

#include "voxel/VoxelSide.h"
uint8_t MortonOctTree::GetVisibleSides(uint32_t x, uint32_t y, uint32_t  z, core::Vector<MNode>::iterator nodeIt)
{
    uint8_t sides=ALL;

    static MNode n;
    n.size = 1;
    n.start = encodeMK(x,y+1,z);

    if (std::binary_search(nodeIt,m_nodes.end(), n, NodeSortPredicate))
        util::RemoveBit(sides, TOP);

    n.start = encodeMK(x,y,z+1);
    if (std::binary_search(nodeIt,m_nodes.end(), n, NodeSortPredicate))
      util::RemoveBit(sides, FRONT);

    n.start = encodeMK(x+1,y,z);
    if (std::binary_search(nodeIt,m_nodes.end(), n, NodeSortPredicate))
      util::RemoveBit(sides, LEFT);

    n.start = encodeMK(x-1,y,z);
    if (std::binary_search(m_nodes.begin(),nodeIt, n, NodeSortPredicate))
      util::RemoveBit(sides, RIGHT);

    n.start = encodeMK(x,y,z-1);
    if (std::binary_search(m_nodes.begin(),nodeIt, n, NodeSortPredicate))
      util::RemoveBit(sides, BACK);

    n.start = encodeMK(x,y-1,z);
    if (std::binary_search(m_nodes.begin(),nodeIt, n, NodeSortPredicate))
      util::RemoveBit(sides, BOTTOM);

    return sides;
}

core::Vector<MNode> & MortonOctTree::GetChildNodes()
{
    return m_nodes;
}