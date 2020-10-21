#include "voxel/world/WorldGenerator.h"
#include "game/Game.h"
#include "util/ChromeTraceLogWriter.h"
#include "util/MultiDimArrayIndex.h"
#include "util/Profiler.h"
#include "util/Timer.h"
#include "util/noise/NoiseGenerator.h"
#include "voxel/MortonOctree.h"
#include "voxel/VoxelUtils.h"
#include "voxel/world/World.h"

namespace gameworld {

WorldGenerator::WorldGenerator(glm::ivec3 halfSizeInChunks, uint32_t seed)
    : m_halfSize(halfSizeInChunks), m_size(halfSizeInChunks * 2), m_seed(seed),
      m_noiseLayerSize(128, 128, 1), m_noiseLayerUID(0) {}

template <class TNumber> uint8_t GetTexture(TNumber x) {
  static_assert(std::is_same<TNumber, double>::value,
                "Passed number must be of type double.");

  if (x < 0.3333) {
    return 204;
  } else if (x < 0.6666) {
    return 1;
  } else {
    return 8;
  }
};

void WorldGenerator::Generate(World *world) {
  ASSERT(m_noiseLayers.size() != 0);

  util::Profiler profiler;

  const auto &nl = m_noiseLayers[0];
  auto chunksCreated = 0;
  auto totalNodesAdded = 0;

  glm::ivec3 currentChunk(-m_halfSize.x, 0, -m_halfSize.z);

  // core::Array<int32_t, World::SuperChunkTotalNodes> array;
  // array.fill(-1);

  glm::ivec3 chunkPos(-m_halfSize.x, 0, -m_halfSize.z);

  auto chunk = world->GetChunk(chunkPos);
  if (!chunk) {
    chunk = world->CreateChunk(chunkPos);
    chunksCreated++;
  }

  chunk->m_nodes.reserve(1048576);

  profiler.Start("Generate");
  for (int32_t chunkZ = -m_halfSize.z; chunkZ < m_halfSize.z; chunkZ++) {
    for (int32_t chunkX = -m_halfSize.x; chunkX < m_halfSize.x; chunkX++) {

      profiler.Start("GenerateXZ");

      chunkPos = glm::ivec3(chunkX, 0, chunkZ);

      profiler.Start("Get or create chunk");
      if (chunkPos != currentChunk) {
        currentChunk = chunkPos;
        // std::fill_n(array, World::SuperChunkTotalNodes, -1);
        chunk = world->CreateChunk(chunkPos);
        chunksCreated++;
      }
      profiler.Stop();

      profiler.Start("Generate noise");
      auto settings = nl.NoiseGenerator->GetSettings();
      auto noiseOffset = glm::ivec3(chunkZ, chunkX, 0) * World::SuperChunkSize;
      settings.Translation =
          core::pod::Vec3<int32_t>(noiseOffset.x, noiseOffset.y, noiseOffset.z);
      nl.NoiseGenerator->SetNoiseGenSettings(settings);
      nl.NoiseGenerator->GenSimplex();
      profiler.Stop();

      profiler.Start("Add nodes to octree");
      /*for (int32_t localChunkZ = 0; localChunkZ < World::SuperChunkSize;
           localChunkZ++) {
        for (int32_t localChunkX = 0; localChunkX < World::SuperChunkSize;
             localChunkX++) {

          auto nval =
              (int)nl.NoiseGenerator->GetNoise(localChunkX, localChunkZ, 0);
          for (int height = 0; height < nval; height++) {
            auto texture = GetTexture(height / 256.0);
            chunk->AddOrphanNode(vox::VoxNode(
                vox::utils::Encode(localChunkX, height, localChunkZ), 1,
                texture.r, texture.g, texture.b));
            totalNodesAdded++;
          }
        }
      }*/
      const uint32_t maxNode = vox::utils::Encode(128,128,128);
      int32_t start = -1, end = -1;
      uint8_t texture;

      for(int32_t i = 0; i < maxNode; i++){
        auto [x,y,z] = vox::utils::Decode(i);
        auto nval =
            (int)nl.NoiseGenerator->GetNoise(x, z, 0);

        if(y <= nval){
          auto t = GetTexture(y / 256.0);

          if(start == -1){
            start = i;
            texture = t;
            continue;
          }
          else if(t != texture){
            chunk->AddOrphanNode(vox::VoxNode(
                start, i - start,
                texture, texture, texture));
            start = -1;
          }
        }
        else if(start != -1){
          chunk->AddOrphanNode(vox::VoxNode(
              start, i - start,
              texture, texture, texture));
          start = -1;
        }
      }

      profiler.Stop();

      profiler.Start("SortLeafNodes");
      chunk->SortLeafNodes();
      profiler.Stop();

      profiler.Stop();

      elog::LogInfo(core::string::format(
          "WorldGen > Chunks created: {}, Nodes added = {}", chunksCreated,
          totalNodesAdded));
    }
  }
  profiler.Stop();

  {
    auto traceFile =
        Game->GetFileSystem()->OpenWrite(io::Path("TraceWorldGen.json"));
    util::ChromeTraceLogWriter logWriter(core::Move(traceFile));
    profiler.WriteLog(logWriter);
    logWriter.FlushLog();
  }
}

void WorldGenerator::AddLayer(core::String name) {
  auto &nl =
      m_noiseLayers.emplace_back(m_noiseLayerUID, name, m_noiseLayerSize);
  auto settings = nl.NoiseGenerator->GetSettings();
  settings.Scale = 128;
  settings.Seed = 6899;
  settings.FractalGain = 0.651f;
  settings.FractalLacunarity = 0;
  settings.FractalOctaves = 10;
  settings.Frequency = 0.003f;
  settings.Offset = 0.012f;
  nl.NoiseGenerator->SetNoiseGenSettings(settings);

  m_noiseLayerUID++;
}
} // namespace gameworld