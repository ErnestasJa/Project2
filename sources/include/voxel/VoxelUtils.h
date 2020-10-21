#ifndef THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
#define THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
#include "OctreeConstants.h"
#include "Morton.h"

namespace vox::utils {
inline uint32_t GetChunk(uint32_t mortonVoxelPosition){
  return mortonVoxelPosition & CHUNK_MASK;
}

inline uint32_t NextChunk(uint32_t mortonVoxelPosition){
  return (mortonVoxelPosition + VOXELS_IN_CHUNK) & CHUNK_MASK;
}

inline std::tuple<uint32_t, uint32_t, uint32_t> Decode(uint32_t mk){
  uint32_t x,y,z;
  decodeMK(mk, x,y,z);
  return {x,y,z};
}

inline uint32_t Encode(uint32_t x, uint32_t y, uint32_t z){
  return encodeMK(x,y,z);
}

inline uint32_t VoxelChunkSpan(const vox::VoxNode& node){
  return node.start + node.size;
}

inline bool IsNodeInChunk(const vox::VoxNode& node, uint32_t chunk){
  return GetChunk(node.start) >= chunk && GetChunk(node.start+node.size-1) <= chunk;
}

inline void FitNodeToChunk(vox::VoxNode& node){
  auto chunkEndForNode = NextChunk(node.start);
  node.size = glm::min(node.size,chunkEndForNode - node.start);
}

inline uint32_t GetNodeEndChunk(const vox::VoxNode& node){
  return GetChunk(node.start+node.size-1);
}

}

#endif // THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
