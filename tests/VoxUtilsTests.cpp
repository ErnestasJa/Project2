#include "voxel/VoxelInc.h"
#include "gtest/gtest.h"

TEST(VoxUtilsTests, test1) {
  vox::VoxNode node(0, 10);

  vox::utils::FitNodeToChunk(node);

  EXPECT_EQ (0,  node.start);
  EXPECT_EQ (10,  node.size);
}

TEST(VoxUtilsTests, test2) {
  vox::VoxNode node(0, vox::VOXELS_IN_CHUNK*10);

  vox::utils::FitNodeToChunk(node);

  EXPECT_EQ (0,  node.start);
  EXPECT_EQ (vox::VOXELS_IN_CHUNK,  node.size);
}