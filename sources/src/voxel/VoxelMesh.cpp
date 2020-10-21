#include "voxel/VoxelMesh.h"
#include "render/IGpuBufferArrayObject.h"
#include "render/IGpuBufferObject.h"

namespace vox {
VoxelMesh::VoxelMesh(core::UniquePtr<render::IGpuBufferArrayObject> vao)
: m_vao(core::Move(vao)), m_isReady(false) {

}
VoxelMesh::~VoxelMesh()
{
}

void VoxelMesh::Upload() {

  if_debug{
    elog::LogInfo(core::string::format("Uploading voxel mesh. Num indices <{}>, Num UVs <{}>, Num vertices <{}>, Num normals: <{}>", Indices.size(), UVs.size(), Vertices.size(), Normals.size()));
  }

  m_vao->GetBufferObject(0)->UpdateBuffer(Indices.size(), Indices.data());
  m_vao->GetBufferObject(1)->UpdateBuffer(UVs.size(), UVs.data());
  m_vao->GetBufferObject(2)->UpdateBuffer(Vertices.size(), Vertices.data());
  m_vao->GetBufferObject(3)->UpdateBuffer(Normals.size(), Normals.data());
  m_isReady = true;
}

void VoxelMesh::Render() {
  ASSERT(m_vao != nullptr, "VAO is empty");
  ASSERT(Vertices.size() == Normals.size(), core::string::format("Num vertices <{}>, Num normals <{}>", Vertices.size(), Normals.size()));
  ASSERT(m_isReady);
  m_vao->Render(Indices.size());
}

void VoxelMesh::Clear() {
  m_isReady = false;

  Indices.clear();
  UVs.clear();
  Vertices.clear();
  Normals.clear();
}
}