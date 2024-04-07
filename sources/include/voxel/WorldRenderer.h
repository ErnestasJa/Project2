#ifndef VOXMESHGENERATOR_H
#define VOXMESHGENERATOR_H

#include "ChunkMesher.h"
#include "VoxelInc.h"
#include "render/RenderFwd.h"
#include "threading/ThreadingInc.h"
#include "util/Bit.h"
#include "voxel/VoxNode.h"
#include "voxel/VoxelFwd.h"
#include "voxel/VoxelMesh.h"
#include <voxel/world/World.h>
namespace vox {
class WorldSubChunk
{
  public:
  WorldSubChunk& operator=(const WorldSubChunk&) = delete;
  WorldSubChunk(const WorldSubChunk&)            = delete;
  WorldSubChunk()                                = default;

  [[nodiscard]] VoxelMesh* GetActiveMesh() const
  {
    return m_isFirstBufferActive ? m_firstMeshBuffer.get() : m_secondMeshBuffer.get();
  }

  [[nodiscard]] VoxelMesh* GetBufferForUpdates() const
  {
    return m_isFirstBufferActive ? m_secondMeshBuffer.get() : m_firstMeshBuffer.get();
  }

  [[nodiscard]] uint32_t GetChunkStartMK() const
  {
    return m_chunkMK;
  }

  void SwapActiveBuffer()
  {
    m_isFirstBufferActive = !m_isFirstBufferActive;
  }

  WorldSubChunk(uint32_t chunkMK, glm::ivec3 chunkOffset,
                core::UniquePtr<vox::VoxelMesh> firstMeshBuffer,
                core::UniquePtr<vox::VoxelMesh> secondMeshBuffer)
      : m_chunkMK(chunkMK)
      , m_chunkOffset(chunkOffset)
      , m_firstMeshBuffer(core::Move(firstMeshBuffer))
      , m_secondMeshBuffer(core::Move(secondMeshBuffer))
      , m_isDirty(false)
      , m_isGenerating(false)
      , m_isFirstBufferActive(true)
  {
    ASSERT(m_firstMeshBuffer && m_secondMeshBuffer);
  }


  private:
  uint32_t                   m_chunkMK;
  glm::ivec3                 m_chunkOffset;
  core::UniquePtr<VoxelMesh> m_firstMeshBuffer;
  core::UniquePtr<VoxelMesh> m_secondMeshBuffer;
  bool                       m_isDirty;
  bool                       m_isGenerating;
  bool                       m_isFirstBufferActive;

  friend class WorldRenderer;
  friend class MesherBackgroundJob;
};

class MesherBackgroundJob : public threading::BackgroundJob
{
  public:
  MesherBackgroundJob(WorldSubChunk* subChunk, std::vector<VoxNode>::iterator chunkStart,
                      std::vector<VoxNode>::iterator chunkEnd)
      : m_subChunk(subChunk)
  {
    subChunk->GetBufferForUpdates()->Clear();
    m_nodesToMesh = core::Vector<VoxNode>(chunkStart, chunkEnd);
  }

  MesherBackgroundJob(WorldSubChunk* subChunk, std::vector<VoxNode>&& chunkNodes)
      : m_subChunk(subChunk)
      , m_nodesToMesh(chunkNodes)
  {
  }

  ~MesherBackgroundJob() override = default;

  void Run() final
  {
    static int chunkCount = 0;
    m_subChunk->GetBufferForUpdates()->Clear();
    chunkMesher.BuildChunk(m_nodesToMesh.begin(), m_nodesToMesh.end(),
                           m_subChunk->GetBufferForUpdates());
    elog::LogInfo(core::string::format("Chunk counter {}", chunkCount));
    chunkCount++;
  }

  void FinalizeInMainThread() final
  {
    m_subChunk->GetBufferForUpdates()->Upload();
    m_subChunk->SwapActiveBuffer();
    m_subChunk->m_isGenerating = false;
  }

  private:
  vox::ChunkMesher     chunkMesher;
  std::vector<VoxNode> m_nodesToMesh;
  WorldSubChunk*       m_subChunk;
};


class WorldRenderer
{
  public:
  static constexpr uint32_t RenderableChunkSize = 32;
  WorldRenderer(
      render::IRenderer* renderer, render::DebugRenderer* debugRenderer, gameworld::World* world,
      vox::EWorldRenderDistance renderDistanceInChunks = vox::EWorldRenderDistance::Medium);

  virtual ~WorldRenderer();

  void SetPlayerOriginInWorld(glm::ivec3 origin);

  void BuildChunkV2(const gw::WorldSuperChunk& chunkData);
  void RenderAllMeshes();

  void Update(float microsecondsElapsed);
  void SetChunkDirty(glm::ivec3 subchunkGlobalOffset);
  void GenerateVisibleChunks();

  void RenderWorldGui();

  private:
  core::Vector<std::tuple<int32_t, gw::WorldSuperChunk*>> GetChunksAroundPlayer();

  core::UniquePtr<vox::VoxelMesh> CreateEmptyMesh();

  WorldSubChunk* GetSubChunk(uint32_t chunkMK, glm::ivec3 worldChunkOffset);

  private:
  // VoxNode m_buildNodes[32][32][32];
  core::SharedPtr<material::BaseMaterial>       m_worldMat;
  core::UnorderedMap<glm::ivec3, WorldSubChunk> m_map;
  gameworld::World*                             m_world;
  render::IRenderer*                            m_renderer;
  render::DebugRenderer*                        m_debugRenderer;
  vox::EWorldRenderDistance                     m_renderDistanceInChunks;
  glm::ivec3                                    m_playerOrigin;
  core::UniquePtr<render::ITexture>             m_worldAtlas;

  threading::BackgroundJobRunner<4> m_backgroundMesher;
};
} // namespace vox

#endif // VOXMESHGENERATOR_H
