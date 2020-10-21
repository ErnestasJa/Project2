#ifndef THEPROJECT2_WORLD_H
#define THEPROJECT2_WORLD_H

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace vox {
class MortonOctree;
}

namespace gameworld {

class World {
public:
  static constexpr int32_t SuperChunkSize = 128;
  static constexpr int32_t SuperChunkTotalNodes = 128 * 128 * 128;
public:
  World() = default;
  ~World() = default;

  vox::MortonOctree* GetChunk(glm::ivec3 chunk){
    auto it = m_worldChunks.find(chunk);
    return it != m_worldChunks.end() ? it->second.get() : nullptr;
  }

  vox::MortonOctree* CreateChunk(glm::ivec3 chunk);

  core::UnorderedMap<glm::ivec3, core::UniquePtr<vox::MortonOctree>> & GetAllChunks(){
    return m_worldChunks;
  }

  core::Vector<std::tuple<int32_t, glm::ivec3, vox::MortonOctree *>>
  GetChunksAroundOrigin(glm::ivec3 originInVoxels, int32_t distanceInSuperChunks);

private:
  core::UnorderedMap<glm::ivec3, core::UniquePtr<vox::MortonOctree>> m_worldChunks;
};
}

namespace gw = gameworld;

#endif // THEPROJECT2_WORLD_H
