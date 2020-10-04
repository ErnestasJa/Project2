#ifndef THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
#define THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
#include "OctreeConstants.h"
#include "Morton.h"

namespace vox::utils {
inline uint32_t GetChunk(uint32_t mortonVoxelPosition){
  return mortonVoxelPosition & CHUNK_MASK;
}

inline uint32_t NextChunk(uint32_t mortonVoxelPosition){
  return (mortonVoxelPosition + VOXELS_IN_CHUNK + 1) & CHUNK_MASK;
}

inline std::tuple<uint32_t, uint32_t, uint32_t> Decode(uint32_t mk){
  uint32_t x,y,z;
  decodeMK(mk, x,y,z);
  return {x,y,z};
}

inline uint32_t Encode(uint32_t x, uint32_t y, uint32_t z){
  return encodeMK(x,y,z);
}

}

#endif // THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
