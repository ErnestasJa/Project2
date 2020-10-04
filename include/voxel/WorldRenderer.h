#ifndef VOXMESHGENERATOR_H
#define VOXMESHGENERATOR_H

#include "ChunkMesher.h"
#include "render/RenderFwd.h"
#include "util/Bit.h"
#include "voxel/VoxNode.h"
#include "voxel/VoxelMesh.h"
#include "voxel/VoxelFwd.h"
#include <mutex>
#include <thread>
#include <voxel/world/World.h>

namespace vox {

class BackgroundJob{
public:
  virtual void Run() = 0;
  ///Override this to execute last steps in the main thread.
  virtual void FinalizeInMainThread() = 0;
  virtual ~BackgroundJob()= default;
};

template <int ConcurrencyLevel = 4>
class BackgroundJobRunner {
public:

  BackgroundJobRunner(){
    m_killAllThreads = false;
    for(int i = 0; i < ConcurrencyLevel; i++){
      m_threads[i] = std::thread(&ThreadRunner, this);
    }
  }

  ~BackgroundJobRunner(){
    JoinAllRunners();
  }

  void EnqueueBackgroundJob(BackgroundJob * backgroundJob){
    m_jobQueueMutex.lock();
    m_backgroundJobQueue.push(core::UniquePtr<BackgroundJob>(backgroundJob));
    m_jobQueueMutex.unlock();
  }

  void Run(){
    if(m_mainThreadFinalizationQueue.empty() == false){
      core::UniquePtr<BackgroundJob> backgroundJob = nullptr;

      m_mainThreadFinalizationQueueMutex.lock();
      backgroundJob = core::Move(m_mainThreadFinalizationQueue.front());
      m_mainThreadFinalizationQueue.pop();
      m_mainThreadFinalizationQueueMutex.unlock();

      ASSERT(backgroundJob);
      backgroundJob->FinalizeInMainThread();
    }
  }

private:
  static void ThreadRunner(BackgroundJobRunner * runner){
    while(runner->m_killAllThreads == false) {
      core::UniquePtr<BackgroundJob> backgroundJobToRun = nullptr;

      if (runner->m_jobQueueMutex.try_lock()) {
        if(runner->m_backgroundJobQueue.empty() == false) {
          backgroundJobToRun = core::Move(runner->m_backgroundJobQueue.front());
          runner->m_backgroundJobQueue.pop();
        }
        runner->m_jobQueueMutex.unlock();
      }

      if (backgroundJobToRun) {
        backgroundJobToRun->Run();

        runner->m_mainThreadFinalizationQueueMutex.lock();
        runner->m_mainThreadFinalizationQueue.push(core::Move(backgroundJobToRun));
        runner->m_mainThreadFinalizationQueueMutex.unlock();
      }
      else{
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }

  void JoinAllRunners(){
    m_killAllThreads = true;
    for(int i = 0; i < ConcurrencyLevel; i++){
      elog::LogInfo(core::string::format("Joining thread <{}>", i));
      m_threads[i].join();
      elog::LogInfo(core::string::format("Joined thread <{}>", i));
    }
  }

  void UpdateJobs(){

  }

private:
  bool m_killAllThreads;
  core::Array<std::thread, ConcurrencyLevel> m_threads;
  std::mutex m_jobQueueMutex;
  std::mutex m_mainThreadFinalizationQueueMutex;
  core::Queue<core::UniquePtr<BackgroundJob>> m_backgroundJobQueue;
  core::Queue<core::UniquePtr<BackgroundJob>> m_mainThreadFinalizationQueue;
};


class WorldSubChunk {
public:
  WorldSubChunk & operator=(const WorldSubChunk &) = delete;
  WorldSubChunk(const WorldSubChunk &) = delete;
  WorldSubChunk() = default;

  VoxelMesh* GetActiveMesh() const {
    return m_isFirstBufferActive ? m_firstMeshBuffer.get() : m_secondMeshBuffer.get();
  }

  VoxelMesh* GetBufferForUpdates() const {
    return m_isFirstBufferActive ? m_secondMeshBuffer.get() : m_firstMeshBuffer.get();
  }

  void SwapActiveBuffer(){
    m_isFirstBufferActive = !m_isFirstBufferActive;
  }

  WorldSubChunk(glm::ivec3 chunkOffset, core::UniquePtr<vox::VoxelMesh> firstMeshBuffer,
                core::UniquePtr<vox::VoxelMesh> secondMeshBuffer)
      : m_chunkOffset(chunkOffset)
      , m_firstMeshBuffer(core::Move(firstMeshBuffer))
      , m_secondMeshBuffer(core::Move(secondMeshBuffer))
      , m_isDirty(false)
      , m_isGenerating(false)
      , m_isFirstBufferActive(true)
  {
    ASSERT(m_firstMeshBuffer && m_secondMeshBuffer);
  }



private:
  glm::ivec3 m_chunkOffset;
  core::UniquePtr<VoxelMesh> m_firstMeshBuffer;
  core::UniquePtr<VoxelMesh> m_secondMeshBuffer;
  bool m_isDirty;
  bool m_isGenerating;
  bool m_isFirstBufferActive;

  friend class WorldRenderer;
  friend class MesherBackgroundJob;
};

class MesherBackgroundJob: public BackgroundJob{
public:
  MesherBackgroundJob(WorldSubChunk* subChunk, std::vector<VoxNode>::iterator chunkStart,
                      std::vector<VoxNode>::iterator chunkEnd):
      m_subChunk(subChunk)
  {
    subChunk->GetBufferForUpdates()->Clear();
    m_nodesToMesh = core::Vector<VoxNode>(chunkStart, chunkEnd);
  }

  ~MesherBackgroundJob() override = default;

  void Run() final{
    chunkMesher.BuildChunk(m_nodesToMesh.begin(), m_nodesToMesh.end(), m_subChunk->GetBufferForUpdates());
  }

  void FinalizeInMainThread() final {
    m_subChunk->GetBufferForUpdates()->Upload();
    m_subChunk->SwapActiveBuffer();
    m_subChunk->m_isGenerating = false;
  }

private:
  vox::ChunkMesher chunkMesher;
  std::vector<VoxNode> m_nodesToMesh;
  WorldSubChunk* m_subChunk;
};


class WorldRenderer {
public:
  static constexpr int32_t RenderableChunkSize = 32;
  WorldRenderer(render::IRenderer *renderer,
                gameworld::World* world, int32_t renderDistanceInChunks = 16);

  virtual ~WorldRenderer();

  void SetPlayerOriginInWorld(glm::ivec3 origin);
  
  void BuildChunk(const std::tuple<glm::ivec3, MortonOctree *> & chunk);
  void RenderAllMeshes();

  void Update(float microsecondsElapsed);
  void SetChunkDirty(glm::ivec3 subchunkGlobalOffset);
  void GenerateVisibleChunks();

  void RenderWorldGui();

private:
  core::Vector<std::tuple<int32_t, glm::ivec3, vox::MortonOctree *>> GetChunksAroundPlayer();

  core::UniquePtr<vox::VoxelMesh>  CreateEmptyMesh();

  std::tuple<glm::ivec3, WorldSubChunk*> GetSubChunk(glm::ivec3 pos){
    auto existingSubChunkIt = m_map.find(pos);

    if(existingSubChunkIt == m_map.end()){
      auto chunkPos = glm::ivec3(pos.x % gw::World::SuperChunkSize, pos.y % gw::World::SuperChunkSize, pos.z % gw::World::SuperChunkSize);

      auto mesh = CreateEmptyMesh();
      auto mesh2 = CreateEmptyMesh();
      auto emplaceResult = m_map.emplace(std::piecewise_construct,
                                         std::forward_as_tuple(pos),
                                         std::forward_as_tuple(chunkPos, core::Move(mesh), core::Move(mesh2)));

      ASSERT(emplaceResult.second, "Failed to insert chunk");

      existingSubChunkIt = emplaceResult.first;
      existingSubChunkIt->second.m_isDirty = true;
    }

    return {
        existingSubChunkIt->first,
        &existingSubChunkIt->second
    };
  }

private:
  //VoxNode m_buildNodes[32][32][32];
  core::SharedPtr<material::BaseMaterial> m_worldMat;
  core::UnorderedMap<glm::ivec3, WorldSubChunk> m_map;
  gameworld::World* m_world;
  render::IRenderer *m_renderer;
  uint32_t m_renderDistanceInChunks;
  glm::ivec3 m_playerOrigin;
  core::UniquePtr<render::ITexture> m_worldAtlas;

  BackgroundJobRunner<6> m_backgroundMesher;
};
}

#endif // VOXMESHGENERATOR_H
