#ifndef THEPROJECT2_SRC_VOXEL_MAP_RANDOMMAPGENERATOR_H_
#define THEPROJECT2_SRC_VOXEL_MAP_RANDOMMAPGENERATOR_H_

#include "voxel/VoxelFwd.h"

namespace vox::map {
class RandomMapGenerator {
public:
  RandomMapGenerator(glm::uvec3 size);
  void Generate(vox::MortonOctree* octree);

protected:
  glm::uvec3 m_size;
};
}

#endif // THEPROJECT2_SRC_VOXEL_MAP_RANDOMMAPGENERATOR_H_
