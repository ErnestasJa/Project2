#ifndef MortonOctree_H
#define	MortonOctree_H

#include "MNode.h"
#include "OctreeConstants.h"
#include "VoxelSide.h"

namespace vox {
class MortonOctree {
public:
  void AddNode(MNode node);
  void AddOrphanNode(MNode node);
  bool IsSorted();
  void SortLeafNodes();
  void RemoveDuplicateNodes();
  bool CheckNodeFloat(float x, float y, float z);
  bool CheckNode(uint32_t x, uint32_t y, uint32_t z);
  uint8_t GetVisibleSides(uint32_t x, uint32_t y, uint32_t z,
                          core::Vector<MNode>::iterator nodeIt);
  core::Vector<MNode> &GetChildNodes();

private:
  core::Vector<MNode> m_nodes;
  void Remove(MNode node);
};
}

#endif	/* MortonOctree_H */
