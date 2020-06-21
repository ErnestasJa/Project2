#include "voxel/VoxelMesh.h"
#include "render/IGpuBufferArrayObject.h"
#include "render/IGpuBufferObject.h"

namespace vox {
VoxelMesh::VoxelMesh(core::UniquePtr<render::IGpuBufferArrayObject> vao)
: m_vao(core::Move(vao)) {

}
VoxelMesh::~VoxelMesh()
{

}

void VoxelMesh::Upload() {
  m_vao->GetBufferObject(0)->UpdateBuffer(Indices.size(), Indices.data());
  m_vao->GetBufferObject(1)->UpdateBuffer(UVs.size(), UVs.data());
  m_vao->GetBufferObject(2)->UpdateBuffer(Vertices.size(), Vertices.data());
}

void VoxelMesh::Render() {
  m_vao->Render(Indices.size());
}
}