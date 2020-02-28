#ifndef COLLISION_INFO_H
#define COLLISION_INFO_H

#include "MNode.h"

namespace vox {
struct CollisionInfo {
  MNode node;
  float nearestDistance;
  glm::vec3 rayStart, rayDirection;

  CollisionInfo(glm::vec3 ray_start, glm::vec3 ray_direction){
    rayStart = ray_start;
    rayDirection = ray_direction;
    nearestDistance = -1;
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