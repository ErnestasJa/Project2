#ifndef EVOXEL_SIDE_H
#define EVOXEL_SIDE_H

#include "utils/Bit.h"
#include <glm/fwd.hpp>

enum VoxelSide
{
    NONE = 0,
    ALL = util::bit<0>() | util::bit<1>() | util::bit<2>() | util::bit<3>() | util::bit<4>() | util::bit<5>(),
    TOP = util::bit<0>(),
    BOTTOM = util::bit<1>(),
    FRONT = util::bit<2>(),
    RIGHT = util::bit<3>(),
    BACK = util::bit<4>(),
    LEFT = util::bit<5>()
};

glm::ivec3 VoxelSideToPosition(VoxelSide side);

#endif