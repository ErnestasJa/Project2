#ifndef THEPROJECT2_CHUNKMESHER_H
#define THEPROJECT2_CHUNKMESHER_H
#include "VoxNode.h"

namespace vox{
class VoxelMesh;
struct MaskNode;
class ChunkMesher {
public:
  ChunkMesher();

  void BuildChunk(core::Vector<VoxNode>::iterator begin, core::Vector<VoxNode>::iterator end, VoxelMesh* voxMesh);

private:
  enum class FacePlane {
    XY = 0, XZ, YZ
  };

private:
  VoxNode GetBuildNode(uint32_t x, uint32_t y, uint32_t z);
  bool CheckBuildNode(uint32_t x, uint32_t y, uint32_t z);
  void SetBuildNode(const VoxNode &node);
  void ClearBuildNodes();

  void BuildSliceMask(uint32_t dir, uint32_t slice, MaskNode mask[32][32]);
  void BuildFacesFromMask(vox::VoxelMesh *mesh, int dim, int z, MaskNode mask[32][32],
                          bool frontFace);

  void AddFaceToMesh(vox::VoxelMesh *mesh, bool frontFace,
                                  FacePlane dir, uint32_t slice,
                                  glm::ivec2 start, glm::ivec2 dims, uint8_t color[3]);

  void AddQuadToMesh(vox::VoxelMesh *mesh, const glm::vec3 *face, glm::ivec2 dims,
                     bool frontFace, FacePlane facePlane, const uint8_t color[3]) noexcept;


  void GreedyBuildChunk(vox::VoxelMesh *mesh);

  uint8_t GetVisibleBuildNodeSides(uint32_t x, uint32_t y, uint32_t z);
  VoxNode m_buildNodes[32][32][32];
};
}

#endif // THEPROJECT2_CHUNKMESHER_H
