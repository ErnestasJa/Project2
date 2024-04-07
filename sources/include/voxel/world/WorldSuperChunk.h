#ifndef THEPROJECTMAIN_WORLDSUPERCHUNK_H
#define THEPROJECTMAIN_WORLDSUPERCHUNK_H

#include "util/TypeUtils.h"
#include "voxel/MortonOctree.h"
#include "voxel/OctreeConstants.h"
#include "voxel/VoxelUtils.h"

namespace gameworld {
struct WorldSuperChunk
{
  private:
  NONCOPYABLE(WorldSuperChunk);

  public:
  using VoxNodeIterator = std::vector<vox::VoxNode>::iterator;

  public:
  WorldSuperChunk(const glm::ivec3& worldPos, core::UniquePtr<vox::MortonOctree> octree)
      : WorldPos(worldPos)
      , Octree(core::Move(octree))
  {
  }

  bool IsEmpty() const
  {
    return Octree->GetNodes().empty();
  }

  VoxNodeIterator GetFirstSubChunk() const
  {
    return Octree->GetNodes().begin();
  }

  VoxNodeIterator GetChunkEnd(VoxNodeIterator it) const
  {
    auto searchVoxNode = vox::VoxNode(vox::utils::NextChunk(it->start + it->size));

    //    elog::LogInfo(core::string::format("searchVoxNode.start  = {}", searchVoxNode.start));

    return std::upper_bound(it, Octree->GetNodes().end(), searchVoxNode,
                            [](const vox::VoxNode& a, const vox::VoxNode& b) {
                              //                              elog::LogInfo(core::string::format("a.start
                              //                              = {}, b.start={}",
                              //                                                                 a.start,
                              //                                                                 b.start));
                              return (a.start & vox::CHUNK_MASK) <= b.start;
                            });
  }

  glm::ivec3                         WorldPos;
  core::UniquePtr<vox::MortonOctree> Octree;
};
} // namespace gameworld

#endif // THEPROJECTMAIN_WORLDSUPERCHUNK_H
