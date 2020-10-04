#ifndef THEPROJECT2_SRC_VOXEL_VOXELMESH_H_
#define THEPROJECT2_SRC_VOXEL_VOXELMESH_H_
#include "render/RenderFwd.h"

namespace vox {
class VoxelMesh {
public:
  core::Vector<uint32_t> Indices;
  core::Vector<glm::vec3> Vertices;
  core::Vector<glm::vec3> UVs;
  core::Vector<glm::vec3> Normals;

public:
  VoxelMesh(core::UniquePtr<render::IGpuBufferArrayObject> vao);
  void Upload();
  void Render();

  void Clear();

  [[nodiscard]] bool IsReady() const {
    return m_isReady;
  }

  ~VoxelMesh();
protected:
  core::UniquePtr<render::IGpuBufferArrayObject> m_vao;
  bool m_isReady;
};
}

#endif // THEPROJECT2_SRC_VOXEL_VOXELMESH_H_
