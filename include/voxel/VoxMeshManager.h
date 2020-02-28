#ifndef VOXMESHGENERATOR_H
#define VOXMESHGENERATOR_H

#include "voxel/VoxelFwd.h"
#include "render/RenderFwd.h"
#include "util/Bit.h"
#include "voxel/MNode.h"

typedef std::unordered_map<uint32_t, core::SharedPtr<vox::VoxelMesh>>::iterator MapIterator;
typedef std::unordered_map<uint32_t, core::SharedPtr<vox::VoxelMesh>> Map;

namespace vox {

class VoxMeshManager {
public:
  VoxMeshManager(render::IRenderer *renderer,
                 core::SharedPtr<MortonOctree> octree, uint32_t level = 5);
  virtual ~VoxMeshManager();

  std::unordered_map<uint32_t, core::SharedPtr<vox::VoxelMesh>> &GetMeshes();
  void RebuildChunk(uint32_t chunk);
  void RenderAllMeshes();
  void GenAllChunks();

private:
  enum class FacePlane {
    XY = 0, XZ, YZ
  };

  void AddVoxelToMesh(vox::VoxelMesh *mesh, const MNode &node, uint8_t sides);
  void AddQuadToMesh(vox::VoxelMesh *mesh, const glm::vec3 *face,
                     bool frontFace, FacePlane facePlane, const uint8_t color[3]) noexcept;
  core::SharedPtr<vox::VoxelMesh> CreateEmptyMesh();
  void ClearMesh(vox::VoxelMesh *mesh);
  void ClearBuildNodes();
  void BuildSliceMask(uint32_t dir, uint32_t slice, MaskNode mask[32][32]);
  void BuildFacesFromMask(vox::VoxelMesh *mesh, int dim, int z,
                          const glm::vec3 &offset, MaskNode mask[32][32],
                          bool frontFaces);
  void AddFaceToMesh(vox::VoxelMesh *mesh, bool frontFace, FacePlane facePlane,
                     uint32_t slice, glm::ivec2 start, glm::ivec2 dims,
                     glm::vec3 offset, uint8_t color[3]);
  void GreedyBuildChunk(vox::VoxelMesh *mesh, const glm::vec3 &offset);
  void SetBuildNode(const MNode &node);
  uint8_t GetVisibleBuildNodeSides(uint32_t x, uint32_t y, uint32_t z);
  MNode GetBuildNode(uint32_t x, uint32_t y, uint32_t z);
  bool CheckBuildNode(uint32_t x, uint32_t y, uint32_t z);

private:
  MNode m_buildNodes[32][32][32];
  core::UnorderedMap<uint32_t, core::SharedPtr<vox::VoxelMesh>> m_map;
  core::SharedPtr<MortonOctree> m_octree;
  render::IRenderer *m_renderer;
  uint32_t m_level;
};
}

#endif // VOXMESHGENERATOR_H
