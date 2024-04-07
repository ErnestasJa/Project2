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

WorldGenerator::WorldGenerator(vox::EWorldSize worldSize, uint32_t seed)
    : m_worldSize(worldSize)
    , m_seed(seed)
    , m_noiseLayerSize(vox::WorldConfig::OctreeSize, vox::WorldConfig::OctreeSize, 1)
    , m_noiseLayerUID(0)
{
}

template <class TNumber> uint8_t GetTexture(TNumber x)
{
  static_assert(std::is_same<TNumber, double>::value, "Passed number must be of type double.");

  if (x < 0.3333)
  {
    return 204;
  }
  else if (x < 0.6666)
  {
    return 1;
  }
  else
  {
    return 8;
  }
};

void WorldGenerator::Generate(World* world)
{
  ASSERT(m_noiseLayers.size() != 0);

  util::Profiler profiler;

  const auto& nl              = m_noiseLayers[0];
  auto        chunksCreated   = 0;
  auto        totalNodesAdded = 0;

  auto halfSize = glm::ivec3((int32_t)m_worldSize / 2);
  auto size     = glm::ivec3((int32_t)m_worldSize);

  glm::ivec3 currentChunk(-halfSize.x, 0, -halfSize.z);
  glm::ivec3 chunkPos(-halfSize.x, 0, -halfSize.z);

  auto chunk = world->GetChunk(chunkPos);
  if (!chunk)
  {
    chunk = world->CreateChunk(chunkPos);
    chunksCreated++;
  }

  chunk->Octree->m_nodes.reserve(1048576);
  util::noise::NoiseGeneratorSettings originalNoiseSettings = nl.NoiseGenerator->GetSettings();

  elog::LogInfo("Start gen");
  profiler.Start("Generate");
  for (int32_t chunkZ = -halfSize.z; chunkZ < halfSize.z; chunkZ++)
  {
    for (int32_t chunkX = -halfSize.x; chunkX < halfSize.x; chunkX++)
    {
      elog::LogInfo(core::string::format("Generating [x={};z={}]", chunkX, chunkZ));
      profiler.Start("GenerateXZ");

      chunkPos = glm::ivec3(chunkX, 0, chunkZ);

      profiler.Start("Get or create chunk");
      if (chunkPos != currentChunk)
      {
        currentChunk = chunkPos;
        // std::fill_n(array, World::SuperChunkTotalNodes, -1);
        chunk = world->CreateChunk(chunkPos);
        chunksCreated++;
      }
      profiler.Stop();

      profiler.Start("Generate noise");
      auto noiseOffset = glm::ivec3(chunkZ, chunkX, 0) * World::SuperChunkSize;

      util::noise::NoiseGeneratorSettings settings = originalNoiseSettings;
      settings.Translation = core::pod::Vec3<int32_t>(noiseOffset.x, noiseOffset.y, noiseOffset.z);
      nl.NoiseGenerator->SetNoiseGenSettings(settings);
      nl.NoiseGenerator->GenSimplex();
      profiler.Stop();

      profiler.Start("Add nodes to octree");

      const uint32_t maxNode = vox::utils::MaxNode(World::SuperChunkSize) + 1;
      int32_t        start = -1, end = -1;
      uint8_t        texture;

      for (int32_t i = 0; i < maxNode; i++)
      {
        auto [x, y, z] = vox::utils::Decode(i);
        auto nval      = (int)nl.NoiseGenerator->GetNoise(x, z, 0);

        if (y <= nval)
        {
          auto t = GetTexture(y / 256.0);
          chunk->Octree->AddOrphanNode(vox::VoxNode(i, 1, t, t, t));
        }
        // this does not work as intended, probably due to mesher issues with bigger size voxels.
        //        if (y <= nval)
        //        {
        //          auto t = GetTexture(y / 256.0);
        //
        //          if (start == -1)
        //          {
        //            start   = i;
        //            texture = t;
        //            continue;
        //          }
        //          else if (t != texture)
        //          {
        //            chunk->Octree->AddOrphanNode(vox::VoxNode(start, i - start, texture, texture,
        //            texture)); start = -1;
        //          }
        //        }
        //        else if (start != -1)
        //        {
        //          chunk->Octree->AddOrphanNode(vox::VoxNode(start, i - start, texture, texture,
        //          texture)); start = -1;
        //        }
      }

      profiler.Stop();

      profiler.Start("SortLeafNodes");
      chunk->Octree->SortLeafNodes();
      profiler.Stop();

      profiler.Stop();

      elog::LogInfo(core::string::format("WorldGen > Chunks created: {}, Nodes added = {}",
                                         chunksCreated, totalNodesAdded));
    }
  }
  profiler.Stop();
  elog::LogInfo("End gen");

  {
    auto traceFile = Game->GetFileSystem()->OpenWrite(io::Path("TraceWorldGen.json"));
    util::ChromeTraceLogWriter logWriter(core::Move(traceFile));
    profiler.WriteLog(logWriter);
    logWriter.FlushLog();
  }
}

void WorldGenerator::AddLayer(core::String name)
{
  auto& nl                   = m_noiseLayers.emplace_back(m_noiseLayerUID, name, m_noiseLayerSize);
  auto  settings             = nl.NoiseGenerator->GetSettings();
  settings.Scale             = 128;
  settings.Seed              = 6899;
  settings.FractalGain       = 0.651f;
  settings.FractalLacunarity = 0;
  settings.FractalOctaves    = 10;
  settings.Frequency         = 0.003f;
  settings.Offset            = 0.012f;
  nl.NoiseGenerator->SetNoiseGenSettings(settings);

  m_noiseLayerUID++;
}
} // namespace gameworld