#include "voxel/map/RandomMapGenerator.h"
#include "voxel/MortonOctree.h"
#include "util/Noise.h"
#include "util/Timer.h"

namespace vox::map {

RandomMapGenerator::RandomMapGenerator(glm::uvec3 size):
m_size(size)
{
}

void RandomMapGenerator::Generate(vox::MortonOctree *octree) {
  util::Timer timer;
  siv::PerlinNoise noise(timer.SecondsSinceEpoch());

  for(uint32_t x = 0; x < m_size.x; x++){
    for(uint32_t z = 0; z < m_size.z; z++){
      uint32_t height = noise.octaveNoise0_1(x / float(m_size.x), z/ float(m_size.z), 16) * m_size.y;

      for(uint32_t y = 0; y < height; y++){
        auto texture = uint32_t(0); //uint32_t((float(y) / float(height)) * 3);
        octree->AddOrphanNode(MNode({x,y,z}, {texture,0,0}));
      }
    }
  }

  octree->SortLeafNodes();
}
}