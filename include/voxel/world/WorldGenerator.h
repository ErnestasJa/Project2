#ifndef THEPROJECT2_SRC_VOXEL_MAP_RANDOMMAPGENERATOR_H_
#define THEPROJECT2_SRC_VOXEL_MAP_RANDOMMAPGENERATOR_H_

#include "util/noise/NoiseGenerator.h"
#include "voxel/VoxelFwd.h"

namespace gameworld {
class World;

struct NoiseLayer {
  NoiseLayer(uint32_t index, core::String name,
             core::pod::Vec3<int32_t> size)
      : Index(index), Name(name), NoiseGenerator(core::MakeUnique<util::noise::NoiseGenerator>(size)) {}

  uint32_t Index;
  core::String Name;
  core::UniquePtr<util::noise::NoiseGenerator> NoiseGenerator;
};

class WorldGenerator {
public:
  WorldGenerator(glm::ivec3 halfSizeInChunks, uint32_t seed = 12345);

  void AddLayer(core::String name);
  void Generate(World *world);

  core::UnorderedMap<core::String, util::noise::NoiseGenerator> &
  GetNoiseLayers();

protected:
  glm::ivec3 m_size;
  glm::ivec3 m_halfSize;
  uint32_t m_seed;
  core::pod::Vec3<int32_t> m_noiseLayerSize;
  core::Vector<NoiseLayer> m_noiseLayers;
  uint32_t m_noiseLayerUID;
};
} // namespace gameworld

#endif // THEPROJECT2_SRC_VOXEL_MAP_RANDOMMAPGENERATOR_H_
