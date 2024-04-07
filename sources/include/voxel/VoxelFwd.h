#ifndef THEPROJECT2_INCLUDE_VOXEL_VOXELFWD_H_
#define THEPROJECT2_INCLUDE_VOXEL_VOXELFWD_H_

namespace vox {
class MortonOctree;
class VoxelMesh;
class WorldRenderer;
class CollisionManager;
struct CollisionInfo;
struct AABBCollisionInfo;
struct MaskNode;
struct VoxNode;

namespace map {
class WorldGenerator;
}

} // namespace vox

#include "VoxelConfig.h"
#include "VoxelSide.h"

#endif // THEPROJECT2_INCLUDE_VOXEL_VOXELFWD_H_
