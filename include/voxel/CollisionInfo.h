#ifndef COLLISION_INFO_H
#define COLLISION_INFO_H

#include "VoxNode.h"

namespace vox {
struct CollisionInfo {
  VoxNode node;
  float nearestDistance;
  glm::vec3 rayStart, rayDirection, rayInverseDirection;

  CollisionInfo(glm::vec3 ray_start, glm::vec3 ray_direction){
    rayStart = ray_start;
    rayDirection = glm::normalize(ray_direction);
    rayInverseDirection = 1.0f/ray_direction;
    nearestDistance = std::numeric_limits<float>::max();
    node.size = -1;
  }

  bool HasCollided(){
    return node.size != -1;
  }
};

struct AABBCollisionInfo {
  uint32_t voxelMK;
  float time;
  glm::vec3 normal;
};
}

#endif