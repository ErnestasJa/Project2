#ifndef THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
#define THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
#include "OctreeConstants.h"

namespace vox::utils {
inline uint32_t GetChunk(uint32_t mortonVoxelPosition){
  return mortonVoxelPosition & CHUNK_MASK;
}
}

#endif // THEPROJECT2_INCLUDE_VOXEL_VOXELUTILS_H_
