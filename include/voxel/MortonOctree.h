#ifndef MortonOctree_H
#define	MortonOctree_H

#include "OctreeConstants.h"
#include "VoxNode.h"
#include "VoxelSide.h"

namespace vox {
class MortonOctree {
public:
  void AddNode(VoxNode node);
  void AddOrphanNode(VoxNode node);
  bool IsSorted();
  void SortLeafNodes();
  void RemoveDuplicateNodes();
  bool CheckNodeFloat(float x, float y, float z);
  bool CheckNode(uint32_t x, uint32_t y, uint32_t z);
  uint8_t GetVisibleSides(uint32_t x, uint32_t y, uint32_t z,
                          core::Vector<VoxNode>::iterator nodeIt);
  core::Vector<VoxNode> &GetNodes();

  bool RemoveNode(uint32_t x, uint32_t y, uint32_t z);

private:
  core::Vector<VoxNode> m_nodes;
  void Remove(VoxNode node);
};
}

#endif	/* MortonOctree_H */
