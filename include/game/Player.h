
#ifndef THEPROJECT2_INCLUDE_GAME_PLAYER_H_
#define THEPROJECT2_INCLUDE_GAME_PLAYER_H_
#include "render/ICamera.h"
#include "voxel/CollisionManager.h"
#include "core/AxisAlignedBoundingBox.h"

namespace game {

class Player {
public:
  Player(core::SharedPtr<render::ICamera> cam, CollisionManager *octree, glm::vec3 position, float width = 1.4f, float height = 2.0f,
         glm::vec3 eyeOffset = glm::vec3(0, 0.5, 0));
  virtual ~Player();

  const core::AxisAlignedBoundingBox &GetAABB();
  glm::vec3 &GetVelocity();
  glm::vec3 &GetPosition();
  glm::vec3 &GetEyeOffset();
  core::SharedPtr<render::ICamera> GetCamera();

  void Update(float timeStep);
  bool Jump(float velocity);
  bool OnGround() const;

  void SetFlyEnabled(bool enabled);
  bool GetFlyEnabled();

protected:
  bool IsColliding();
  bool IsOnGround();
  bool IsSweptColliding(float timeStep);

  CollisionManager *m_octree;
  core::SharedPtr<render::ICamera> m_cam;
  glm::vec3 m_eyeOffset, m_position, m_velocity;
  core::AxisAlignedBoundingBox m_aabb;
  bool m_onGround, m_flyEnabled;
};
}
#endif // THEPROJECT2_INCLUDE_GAME_PLAYER_H_
