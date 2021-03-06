#include "CollisionInfo.h"

namespace vox {
CollisionInfo::CollisionInfo(glm::vec3 ray_start, glm::vec3 ray_direction) {
  nearestDistance = INFINITY;
  rayStart = ray_start;
  rayDirection = ray_direction;
  helpers::invert(rayDirection);
}
}