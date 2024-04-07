#ifndef THEPROJECTMAIN_VOXELCONFIG_H
#define THEPROJECTMAIN_VOXELCONFIG_H
namespace vox {
enum class EWorldRenderDistance : uint32_t
{
  Small   = 16,
  Medium  = 24,
  High    = 32,
  Extreme = 64,
  Insane  = 128
};

/// World size in superchunks
enum class EWorldSize : uint32_t
{
  Small     = 4,
  Medium    = 16,
  High      = 32,
  Extreme   = 64,
  Insane    = 128,
  Ludicrous = 256
};

struct WorldConfig
{
  // size of the whole octree
  static constexpr uint32_t OctreeSize = 128;
  // size of mesh contained inside of the octree
  static constexpr uint32_t MeshSize = 32;

  static constexpr EWorldSize WorldSizeInSuperChunks = EWorldSize::Small;

  static_assert(OctreeSize % MeshSize == 0, "Size of octree should be multiple of mesh size");
};
} // namespace vox
#endif // THEPROJECTMAIN_VOXELCONFIG_H
