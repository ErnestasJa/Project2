#include "voxel/world/World.h"
#include "voxel/MortonOctree.h"

namespace gameworld {

vox::MortonOctree *World::CreateChunk(glm::ivec3 chunk) {
  auto octree = new vox::MortonOctree();
  m_worldChunks.emplace(
      std::piecewise_construct, std::forward_as_tuple(chunk),
      std::forward_as_tuple(core::UniquePtr<vox::MortonOctree>(octree)));
  return octree;
}

core::Vector<std::tuple<int32_t, glm::ivec3, vox::MortonOctree *>> World::GetChunksAroundOrigin(glm::ivec3 originInVoxels, int32_t distanceInSuperChunks) {
  core::Vector<std::tuple<int32_t, glm::ivec3, vox::MortonOctree *>> superChunks;

  glm::ivec3 playerSuperChunk = glm::ivec3(originInVoxels.x/SuperChunkSize,
                                           originInVoxels.y/SuperChunkSize,
                                           originInVoxels.z/SuperChunkSize);


  glm::ivec3 renderMinChunk = playerSuperChunk - glm::ivec3(distanceInSuperChunks);
  glm::ivec3 renderMaxChunk = playerSuperChunk + glm::ivec3(distanceInSuperChunks);



  for(int32_t y = renderMinChunk.y; y <= renderMaxChunk.y; y++){
    for(int32_t z = renderMinChunk.z; z <= renderMaxChunk.z; z++){
      for(int32_t x = renderMinChunk.x; x <= renderMaxChunk.x; x++){
        auto pos = glm::ivec3(x,y,z);
        auto superChunk = GetChunk(pos);

        if(superChunk) {
          auto distance = (int)glm::floor(glm::distance(
              glm::vec3(pos), glm::vec3(playerSuperChunk)));

          superChunks.push_back(std::make_tuple(distance, pos * SuperChunkSize, superChunk));
        }
      }
    }
  }

  std::sort(std::begin(superChunks), std::end(superChunks), [](const std::tuple<int32_t, glm::ivec3, vox::MortonOctree *> & a, const std::tuple<int32_t, glm::ivec3, vox::MortonOctree *> & b){
      return std::get<0>(a) < std::get<0>(b);
  });

  return superChunks;
}

} // namespace gameworld