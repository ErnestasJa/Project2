#ifndef THEPROJECT2_WORLD_H
#define THEPROJECT2_WORLD_H

#define GLM_ENABLE_EXPERIMENTAL

#include "WorldSuperChunk.h"
#include "glm/gtx/hash.hpp"

namespace gameworld {

class World
{
  public:
  static constexpr int32_t SuperChunkSize       = 128;
  static constexpr int32_t SuperChunkTotalNodes = 128 * 128 * 128;

  public:
  World()  = default;
  ~World() = default;

  WorldSuperChunk* GetChunk(glm::ivec3 chunk)
  {
    auto it = m_worldChunks.find(chunk);
    return it != m_worldChunks.end() ? &it->second : nullptr;
  }

  WorldSuperChunk* CreateChunk(glm::ivec3 chunk);

  core::UnorderedMap<glm::ivec3, WorldSuperChunk>& GetAllChunks()
  {
    return m_worldChunks;
  }

  core::Vector<std::tuple<int32_t, WorldSuperChunk*>> GetChunksAroundOrigin(
      glm::ivec3 originInVoxels, int32_t distanceInSuperChunks);

  private:
  core::UnorderedMap<glm::ivec3, WorldSuperChunk> m_worldChunks;
};
} // namespace gameworld

namespace gw = gameworld;

#endif // THEPROJECT2_WORLD_H
