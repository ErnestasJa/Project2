#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

namespace core {
class AxisAlignedBoundingBox;
}

struct CollisionInfo;
struct AABBCollisionInfo;
class MortonOctTree;
typedef core::SharedPtr<MortonOctTree> MortonOctTreePtr;

#include "VoxelSide.h"

class CollisionManager
{
    public:
        CollisionManager(MortonOctTreePtr octree);
        virtual ~CollisionManager();

    	bool CheckCollision(const glm::vec3 & bmin, const glm::vec3 & bmax, const glm::vec3 & rayStart, const glm::vec3 & rayDirectionInverse);
    	bool CheckCollision(const core::AxisAlignedBoundingBox & aabb);
    	bool CheckCollisionB(const core::AxisAlignedBoundingBox & aabb);
        core::Vector<AABBCollisionInfo> CheckCollisionSwept(const core::AxisAlignedBoundingBox & aabb, const glm::vec3 & vel);
    	void Collide(CollisionInfo & colInfo, uint32_t depthLevel, const glm::ivec3 & octStart);
    	VoxelSide GetCollisionSide(glm::vec3 voxPos, glm::vec3 rayStart,  glm::vec3 rayDirection);

    protected:
        MortonOctTreePtr m_octree;
        uint32_t Depth; /// just until we get rid of templated octree.
};

#endif // COLLISIONMANAGER_H
