#include "voxel/VoxelInc.h"
#include "gtest/gtest.h"

TEST(VoxUtilsTests, test1)
{
  vox::VoxNode node(0, 10);

  vox::utils::FitNodeToChunk(node);

  EXPECT_EQ(0, node.start);
  EXPECT_EQ(10, node.size);
}

TEST(VoxUtilsTests, test2)
{
  vox::VoxNode node(0, vox::VOXELS_IN_CHUNK * 10);

  vox::utils::FitNodeToChunk(node);

  EXPECT_EQ(0, node.start);
  EXPECT_EQ(vox::VOXELS_IN_CHUNK, node.size);
}

TEST(VoxUtilsTests, MortonIterationStaysInWorldBounds)
{
  const uint32_t maxNode = vox::utils::Encode(127, 127, 127) + 1;

  for (int32_t i = 0; i < maxNode; i++)
  {
    auto [x, y, z] = vox::utils::Decode(i);
    if (!(x < 128 && y < 128 && z < 128))
    {
      core::String msg = core::string::format(
          "Expected all values to be < 128 \n3D index = [{},{},{}]\nMorton key: {}\n", x, y, z, i);
      auto [px, py, pz] = vox::utils::Decode(i - 1);
      msg += core::string::format("previous morton value: {} \n3D index = [{},{},{}]\n", i - 1, px,
                                  py, pz);
      FAIL() << msg;
    }

    SUCCEED() << "";
  }
}