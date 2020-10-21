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

#include "gui/IGui.h"

namespace vox {
WorldRenderer::WorldRenderer(render::IRenderer *renderer, gw::World *world,
                             int32_t renderDistanceInChunks)
    : m_renderer(renderer), m_world(world),
      m_renderDistanceInChunks(renderDistanceInChunks),
      m_playerOrigin(0, 0, 0) {

  m_worldMat =
      Game->GetResourceManager()->LoadMaterial("resources/shaders/voxel");
  m_worldAtlas = Game->GetImageLoader()->LoadAtlasAs2DTexture(
      io::Path("resources/textures/block_atlas.png"), 48);
  m_worldMat->SetTexture(0, m_worldAtlas.get());
}

WorldRenderer::~WorldRenderer() {}

core::UniquePtr<vox::VoxelMesh> WorldRenderer::CreateEmptyMesh() {
  core::Vector<render::BufferDescriptor> bufferDescriptors = {
      render::BufferDescriptor{1, render::BufferObjectType::index,
                               render::BufferComponentDataType::uint32, 0},

      render::BufferDescriptor{3, render::BufferObjectType::vertex,
                               render::BufferComponentDataType::float32, 0},

      render::BufferDescriptor{3, render::BufferObjectType::vertex,
                               render::BufferComponentDataType::float32, 1},
      render::BufferDescriptor{3, render::BufferObjectType::vertex,
                               render::BufferComponentDataType::float32, 2},
  };

  auto vao = m_renderer->CreateBufferArrayObject(bufferDescriptors);
  return core::MakeUnique<vox::VoxelMesh>(core::Move(vao));
}

void WorldRenderer::BuildChunk(
    const std::tuple<glm::ivec3, MortonOctree *> &chunkData) {

  auto superChunkOffset = std::get<0>(chunkData);
  vox::MortonOctree *chunk = std::get<1>(chunkData);
  auto &nodes = chunk->GetNodes();

  if (nodes.empty()) {
    return;
  }

  /*int32_t index = 0;
  auto currentChunk = vox::utils::GetChunk(nodes[0].start);
  auto chunkStartIndex = 0;

  vox::VoxNode extraNode(0, 0);

  while(true){
    const auto& currentNode = nodes[index];
    auto currentNodeChunkStart = vox::utils::GetChunk(currentNode.start);
    auto currentNodeChunkEnd = vox::utils::GetNodeEndChunk(currentNode);

    if(currentNodeChunkStart != currentChunk){
      const auto & firstVoxelIt = nodes.begin() + chunkStartIndex;
      const auto & lastVoxelIt = nodes.begin() + (index-1);

      uint32_t firstVoxelChunk = vox::utils::GetChunk(firstVoxelIt->start);
      uint32_t lastVoxelChunk = vox::utils::GetChunk(lastVoxelIt->start + lastVoxelIt->size-1);

      auto [cx, cy, cz] = vox::utils::Decode(firstVoxelChunk);

      glm::ivec3 chunkOffset(cx, cy, cz);

      auto renderableChunkOffset = glm::ivec3(superChunkOffset + chunkOffset);
      auto existingSubChunkIt = GetSubChunk(firstVoxelChunk, renderableChunkOffset);
      auto &worldSubChunk = std::get<1>(existingSubChunkIt);

      core::Vector<vox::VoxNode> chunkNodes(firstVoxelIt, lastVoxelIt);

      m_backgroundMesher.EnqueueBackgroundJob(new MesherBackgroundJob(
          worldSubChunk, core::Move(chunkNodes)));


    }

    index++;
    if(index == nodes.size()){
      break;
    }
  }*/

  auto firstVoxelInChunkIt = nodes.begin();
  auto chunkStartMK = vox::utils::GetChunk(nodes.begin()->start);

  while (true) {
    auto nextChunkMk = vox::utils::NextChunk(chunkStartMK);

    auto searchVoxNode = vox::VoxNode(nextChunkMk);
    auto nextChunkIt =
        std::upper_bound(firstVoxelInChunkIt, nodes.end(), searchVoxNode,
                         [](const vox::VoxNode &a, const vox::VoxNode &b) {
                           return (a.start & CHUNK_MASK) <= b.start;
                         });

    auto lastVoxelInChunkIt = nextChunkIt - 1;

    auto worldSubChunk = GetSubChunk(chunkStartMK, superChunkOffset);
    auto lastVoxelEndChunkMk = vox::utils::GetChunk(lastVoxelInChunkIt->start + lastVoxelInChunkIt->size);
    auto numNodes = std::distance(firstVoxelInChunkIt, lastVoxelInChunkIt);

    if (worldSubChunk->m_isDirty && worldSubChunk->m_isGenerating == false && numNodes > 0) {
      worldSubChunk->m_isGenerating = true;

      //TODO: if last node in chunk overflows change size to fit in current chunk.

      core::Vector<vox::VoxNode> chunkNodes(firstVoxelInChunkIt, lastVoxelInChunkIt);
      if(chunkStartMK != lastVoxelEndChunkMk) {
        auto& lastNode = chunkNodes[chunkNodes.size()-1];

      }

      m_backgroundMesher.EnqueueBackgroundJob(new MesherBackgroundJob(
          worldSubChunk, firstVoxelInChunkIt, lastVoxelInChunkIt));
    }

    if(chunkStartMK != lastVoxelEndChunkMk){
      auto nextChunkMK = vox::utils::NextChunk(chunkStartMK);

      while(lastVoxelEndChunkMk != nextChunkMK){
        auto subChunk = GetSubChunk(nextChunkMK, superChunkOffset);
        core::Vector<vox::VoxNode> chunkNodes = { vox::VoxNode(nextChunkMK, vox::utils::NextChunk(nextChunkMK)-1) };

        m_backgroundMesher.EnqueueBackgroundJob(new MesherBackgroundJob(
            subChunk, core::Move(chunkNodes)));

        nextChunkMK = vox::utils::NextChunk(chunkStartMK);
      }

      //TODO: deal with the leftover voxel
    }

    if (nextChunkIt == nodes.end()) {
      break;
    } else {
      firstVoxelInChunkIt = nextChunkIt;
      chunkStartMK = vox::utils::GetChunk(nextChunkIt->start);
    }
  }
}

float g_LightPower = 640000;
glm::vec3 g_LightPosition = glm::vec3(200, 656, 400);

void WorldRenderer::RenderWorldGui() {
  ImGui::Begin("Player settings");
  ImGui::DragFloat("Light power", &g_LightPower, 25);
  ImGui::DragFloat3("Light position", &g_LightPosition.x, 25);
  ImGui::End();
}

void WorldRenderer::RenderAllMeshes() {
  auto cam = m_renderer->GetRenderContext()->GetCurrentCamera();
  m_worldMat->Use();
  Game->GetRenderer()->GetRenderContext()->SetDepthTest(true);
  Game->GetRenderer()->SetActiveTextures(m_worldMat->GetTextures());

  for (auto &it : m_map) {
    auto mesh = it.second.GetActiveMesh();
    ASSERT(mesh != nullptr, "Mesh is null");

    if(mesh->IsReady() == false){
      continue;
    }

    auto model = glm::translate(glm::mat4(1), glm::vec3(it.first));
    m_worldMat->SetMat4("MVP", cam->GetProjection() * cam->GetView() * model);
    m_worldMat->SetMat4("M", model);
    m_worldMat->SetMat4("V", cam->GetView());

    m_worldMat->SetVec3("LightPosition_worldspace", g_LightPosition);
    m_worldMat->SetF("LightPower", g_LightPower);

    mesh->Render();
  }
}
void WorldRenderer::SetPlayerOriginInWorld(glm::ivec3 origin) {
  m_playerOrigin = origin;
}

void WorldRenderer::GenerateVisibleChunks() {
  auto chunksAroundPlayer = GetChunksAroundPlayer();

  for (auto &chunkData : chunksAroundPlayer) {
    BuildChunk(std::make_tuple(std::get<1>(chunkData), std::get<2>(chunkData)));
  }
}

core::Vector<std::tuple<int32_t, glm::ivec3, vox::MortonOctree *>>
WorldRenderer::GetChunksAroundPlayer() {
  glm::ivec3 playerSuperChunk =
      glm::ivec3(m_playerOrigin.x / gw::World::SuperChunkSize,
                 m_playerOrigin.y / gw::World::SuperChunkSize,
                 m_playerOrigin.z / gw::World::SuperChunkSize);

  auto renderDistanceInVoxels = m_renderDistanceInChunks * RenderableChunkSize;
  auto renderDistanceInSuperChunks =
      (renderDistanceInVoxels / gw::World::SuperChunkSize) + 1;

  return m_world->GetChunksAroundOrigin(m_playerOrigin,
                                        renderDistanceInSuperChunks);
}

void WorldRenderer::Update(float microsecondsElapsed) {
  m_backgroundMesher.Run();
  //GenerateVisibleChunks();
}


void WorldRenderer::SetChunkDirty(glm::ivec3 subchunkGlobalOffset) {
  auto it = m_map.find(subchunkGlobalOffset);
  if (it != m_map.end()) {
    it->second.m_isDirty = true;
  }
}

WorldSubChunk* WorldRenderer::GetSubChunk(uint32_t chunkMK, glm::ivec3 worldChunkOffset){
  auto [cx, cy, cz] = vox::utils::Decode(chunkMK);
  auto pos = worldChunkOffset + glm::ivec3(cx,cy,cz);
  auto existingSubChunkIt = m_map.find(pos);

  if(existingSubChunkIt == m_map.end()){
    auto chunkPos = glm::ivec3(pos.x % gw::World::SuperChunkSize, pos.y % gw::World::SuperChunkSize, pos.z % gw::World::SuperChunkSize);

    auto mesh = CreateEmptyMesh();
    auto mesh2 = CreateEmptyMesh();
    auto emplaceResult = m_map.emplace(std::piecewise_construct,
                                       std::forward_as_tuple(pos),
                                       std::forward_as_tuple(chunkMK, chunkPos, core::Move(mesh), core::Move(mesh2)));

    ASSERT(emplaceResult.second, "Failed to insert chunk");

    existingSubChunkIt = emplaceResult.first;
    existingSubChunkIt->second.m_isDirty = true;
  }

  return &existingSubChunkIt->second;
}

} // namespace vox