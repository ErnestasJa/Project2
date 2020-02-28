#include "voxel/map/RandomMapGenerator.h"
#include "voxel/MortonOctree.h"
#include "util/Noise.h"
#include "util/Timer.h"

namespace vox::map {

RandomMapGenerator::RandomMapGenerator(glm::uvec3 size):
m_size(size)
{
}

core::pod::Vec3<uint8_t> GetTexture(double x){
  if(x < 0.3333){
    return core::pod::Vec3<uint8_t>(204, 3,2);
  }
  else if(x < 0.6666){
    return core::pod::Vec3<uint8_t>(1, 1,1);
  }
  else
  {
    return core::pod::Vec3<uint8_t>(8, 8,8);
  }
};

void RandomGen(vox::MortonOctree *octree, glm::uvec3 size){
  util::Timer timer;
  siv::PerlinNoise noise(timer.SecondsSinceEpoch());
  siv::PerlinNoise blockNoise(timer.SecondsSinceEpoch() + 1000);

  for(uint32_t x = 0; x < size.x; x++){
    for(uint32_t z = 0; z < size.z; z++){
      uint32_t height = noise.octaveNoise0_1(x / float(size.x), z/ float(size.z), 16) * size.y;
      double block = blockNoise.octaveNoise0_1(x / float(size.x), z/ float(size.z), 16);
      auto tex = GetTexture(block);

      for(uint32_t y = 0; y < height; y++){
        auto texture = uint32_t(0); //uint32_t((float(y) / float(height)) * 3);
        octree->AddOrphanNode(MNode({x,y,z}, tex));
      }
    }
  }

  octree->SortLeafNodes();
}

void SingleBlock(vox::MortonOctree *octree){
  octree->AddOrphanNode(MNode({0,0,0}, GetTexture(0)));
  octree->SortLeafNodes();
}

void RandomMapGenerator::Generate(vox::MortonOctree *octree) {
  //SingleBlock(octree);
  RandomGen(octree, m_size);
}
}