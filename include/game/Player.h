#ifndef THEPROJECT2_INCLUDE_GAME_PLAYER_H_
#define THEPROJECT2_INCLUDE_GAME_PLAYER_H_
#include "core/AxisAlignedBoundingBox.h"
#include "render/ICamera.h"
#include "voxel/CollisionManager.h"
#include <object/AnimatedMeshActor.h>

namespace render{
class DebugRenderer;
}

namespace game {
class Player {
public:
  Player(render::DebugRenderer* debugRenderer, core::SharedPtr<game::obj::AnimatedMeshActor> playerActor,
      core::SharedPtr<render::ICamera> cam, vox::CollisionManager *octree,
      glm::vec3 position, float width = 1.4f, float height = 1.98f,
         glm::vec3 eyeOffset = glm::vec3(0, 0.9, 0));

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
  render::DebugRenderer* m_debugRenderer;
  vox::CollisionManager *m_octree;
  core::SharedPtr<render::ICamera> m_cam;
  core::SharedPtr<game::obj::AnimatedMeshActor> m_playerActor;
  glm::vec3 m_eyeOffset, m_position, m_velocity;
  core::AxisAlignedBoundingBox m_aabb;
  float m_height;
  bool m_onGround, m_flyEnabled;
  void UpdateMesh(float deltaSeconds);
};
}
#endif // THEPROJECT2_INCLUDE_GAME_PLAYER_H_
