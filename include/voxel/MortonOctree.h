#ifndef MORTONOCTTREE_H
#define	MORTONOCTTREE_H

#include "MNode.h"
#include "OctreeConstants.h"
#include "VoxelSide.h"

class MortonOctTree
{
public:
    void AddNode(MNode node);
    void AddOrphanNode(MNode node);
    bool IsSorted();
    void SortLeafNodes();
    void RemoveDuplicateNodes();
    bool CheckNodeFloat(float x, float y, float z);
    bool CheckNode(uint32_t x, uint32_t y, uint32_t z);
    uint8_t GetVisibleSides(uint32_t x, uint32_t y, uint32_t  z, core::Vector<MNode>::iterator nodeIt);
    core::Vector<MNode> & GetChildNodes();

private:
  core::Vector<MNode> m_nodes;
};

typedef core::SharedPtr<MortonOctTree> MortonOctTreePtr;

#endif	/* MORTONOCTTREE_H */
