#include "voxel/WorldRenderer.h"
#include "game/Game.h"
#include "stdlib.h"
#include "voxel/ChunkMesher.h"
#include "voxel/Morton.h"
#include "voxel/MortonOctree.h"
#include "voxel/OctreeConstants.h"
#include "voxel/VoxelMesh.h"
#include "voxel/VoxelUtils.h"
#include "voxel/world/World.h"
#include <render/BaseMesh.h>
#include <render/IGpuBufferArrayObject.h>
#include <render/IRenderer.h>
#include <render/debug/DebugRenderer.h>

#include "gui/IGui.h"

namespace vox {
WorldRenderer::WorldRenderer(render::IRenderer* renderer, render::DebugRenderer* debugRenderer,
                             gw::World* world, vox::EWorldRenderDistance renderDistanceInChunks)
    : m_renderer(renderer)
    , m_world(world)
    , m_renderDistanceInChunks(renderDistanceInChunks)
    , m_playerOrigin(0, 0, 0)
{

  m_worldMat = Game->GetResourceManager()->LoadMaterial("resources/shaders/voxel");
  m_worldAtlas =
      Game->GetImageLoader()->LoadAtlasAs2DTexture(io::Path("resources/textures/block_atlas.png"),
                                                   48);
  m_worldMat->SetTexture(0, m_worldAtlas.get());
  m_debugRenderer = debugRenderer;
}

WorldRenderer::~WorldRenderer() {}

core::UniquePtr<vox::VoxelMesh> WorldRenderer::CreateEmptyMesh()
{
  core::Vector<render::BufferDescriptor> bufferDescriptors = {
    render::BufferDescriptor{ 1, render::BufferObjectType::index,
                              render::BufferComponentDataType::uint32, 0 },

    render::BufferDescriptor{ 3, render::BufferObjectType::vertex,
                              render::BufferComponentDataType::float32, 0 },

    render::BufferDescriptor{ 3, render::BufferObjectType::vertex,
                              render::BufferComponentDataType::float32, 1 },
    render::BufferDescriptor{ 3, render::BufferObjectType::vertex,
                              render::BufferComponentDataType::float32, 2 },
  };

  auto vao = m_renderer->CreateBufferArrayObject(bufferDescriptors);
  return core::MakeUnique<vox::VoxelMesh>(core::Move(vao));
}

void WorldRenderer::BuildChunkV2(const gw::WorldSuperChunk& chunkData)
{
  if (chunkData.IsEmpty())
  {
    return;
  }

  auto superChunkOffset = chunkData.WorldPos;

  auto firstVoxelInChunkIt = chunkData.GetFirstSubChunk();
  auto lastVoxelInChunkIt  = chunkData.GetChunkEnd(firstVoxelInChunkIt);

  elog::LogInfo("\nBuildChunkV2 start\n");

  while (true)
  {
    auto [x, y, z] = vox::utils::Decode(vox::utils::GetChunk(firstVoxelInChunkIt->start));
    elog::LogInfo(core::string::format("subchunk offset: [{},{},{}], superchunk: [{},{},{}]", x, y,
                                       z, superChunkOffset.x, superChunkOffset.y,
                                       superChunkOffset.z));

    auto worldSubChunk =
        GetSubChunk(vox::utils::GetChunk(firstVoxelInChunkIt->start), superChunkOffset);

    if (worldSubChunk->m_isDirty && worldSubChunk->m_isGenerating == false)
    {
      worldSubChunk->m_isGenerating = true;

      core::Vector<vox::VoxNode> chunkNodes(firstVoxelInChunkIt, lastVoxelInChunkIt);

      m_backgroundMesher.EnqueueBackgroundJob(
          new MesherBackgroundJob(worldSubChunk, firstVoxelInChunkIt, lastVoxelInChunkIt));
    }

    if (lastVoxelInChunkIt == chunkData.Octree->GetNodes().end())
    {
      break;
    }

    firstVoxelInChunkIt = lastVoxelInChunkIt + 1;

    lastVoxelInChunkIt = chunkData.GetChunkEnd(firstVoxelInChunkIt);
  }
  elog::LogInfo("\nBuildChunkV2 end\n");
}

float     g_LightPower    = 640000;
glm::vec3 g_LightPosition = glm::vec3(200, 656, 400);

void WorldRenderer::RenderWorldGui()
{
  ImGui::Begin("Player settings");
  ImGui::DragFloat("Light power", &g_LightPower, 25);
  ImGui::DragFloat3("Light position", &g_LightPosition.x, 25);
  ImGui::End();
}

void WorldRenderer::RenderAllMeshes()
{
  auto cam = m_renderer->GetRenderContext()->GetCurrentCamera();
  m_worldMat->Use();
  Game->GetRenderer()->GetRenderContext()->SetDepthTest(true);
  Game->GetRenderer()->SetActiveTextures(m_worldMat->GetTextures());

  elog::LogInfo("\nStart render world:");

  for (auto& it : m_map)
  {
    auto mesh = it.second.GetActiveMesh();
    ASSERT(mesh != nullptr, "Mesh is null");

    if (mesh->IsReady() == false)
    {
      continue;
    }

    auto model = glm::translate(glm::mat4(1), glm::vec3(it.first));
    m_worldMat->SetMat4("MVP", cam->GetProjection() * cam->GetView() * model);
    m_worldMat->SetMat4("M", model);
    m_worldMat->SetMat4("V", cam->GetView());

    m_worldMat->SetVec3("LightPosition_worldspace", g_LightPosition);
    m_worldMat->SetF("LightPower", g_LightPower);

    mesh->Render();
    m_debugRenderer->AddAABV(glm::vec3(it.first), glm::vec3(vox::WorldConfig::MeshSize), 0.5);

    elog::LogInfo(core::string::format("Mesh [{},{},{}]", it.first.x, it.first.y, it.first.z));
  }

  elog::LogInfo("\nEnd render world\n");
}
void WorldRenderer::SetPlayerOriginInWorld(glm::ivec3 origin)
{
  m_playerOrigin = origin;
}

void WorldRenderer::GenerateVisibleChunks()
{
  auto chunksAroundPlayer = GetChunksAroundPlayer();

  for (auto& chunkData : chunksAroundPlayer)
  {
    BuildChunkV2(*std::get<1>(chunkData));
  }
}

core::Vector<std::tuple<int32_t, gw::WorldSuperChunk*>> WorldRenderer::GetChunksAroundPlayer()
{
  glm::ivec3 playerSuperChunk = glm::ivec3(m_playerOrigin.x / gw::World::SuperChunkSize,
                                           m_playerOrigin.y / gw::World::SuperChunkSize,
                                           m_playerOrigin.z / gw::World::SuperChunkSize);

  auto renderDistanceInVoxels      = uint32_t(m_renderDistanceInChunks) * RenderableChunkSize;
  auto renderDistanceInSuperChunks = (renderDistanceInVoxels / gw::World::SuperChunkSize) + 1;

  return m_world->GetChunksAroundOrigin(m_playerOrigin, renderDistanceInSuperChunks);
}

void WorldRenderer::Update(float microsecondsElapsed)
{
  m_backgroundMesher.Run();
  // GenerateVisibleChunks();
}


void WorldRenderer::SetChunkDirty(glm::ivec3 subchunkGlobalOffset)
{
  auto it = m_map.find(subchunkGlobalOffset);
  if (it != m_map.end())
  {
    it->second.m_isDirty = true;
  }
}

WorldSubChunk* WorldRenderer::GetSubChunk(uint32_t chunkMK, glm::ivec3 worldChunkOffset)
{
  auto [cx, cy, cz] = vox::utils::Decode(chunkMK);
  auto pos = worldChunkOffset * glm::ivec3(vox::WorldConfig::OctreeSize) + glm::ivec3(cx, cy, cz);
  auto existingSubChunkIt = m_map.find(pos);

  if (existingSubChunkIt == m_map.end())
  {
    auto chunkPos = glm::ivec3(pos.x % gw::World::SuperChunkSize, pos.y % gw::World::SuperChunkSize,
                               pos.z % gw::World::SuperChunkSize);

    auto mesh          = CreateEmptyMesh();
    auto mesh2         = CreateEmptyMesh();
    auto emplaceResult = m_map.emplace(std::piecewise_construct, std::forward_as_tuple(pos),
                                       std::forward_as_tuple(chunkMK, chunkPos, core::Move(mesh),
                                                             core::Move(mesh2)));

    ASSERT(emplaceResult.second, "Failed to insert chunk");

    existingSubChunkIt                   = emplaceResult.first;
    existingSubChunkIt->second.m_isDirty = true;
  }

  return &existingSubChunkIt->second;
}

} // namespace vox