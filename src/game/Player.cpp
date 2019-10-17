#include "game/Player.h"
#include "voxel/CollisionInfo.h"
#include "voxel/CollisionManager.h"
#include <core/AxisAlignedBoundingBox.h>
#include <glm/geometric.hpp>

namespace game {
static constexpr float GRAVITY_CONSTANT = -4.8f;
static constexpr float TERMINAL_VELOCITY = -20.0f;

Player::Player(core::SharedPtr<render::ICamera> cam, CollisionManager *octree,
               glm::vec3 position, float width, float height,
               glm::vec3 eyeOffset) {
  m_octree = octree;
  m_cam = cam;
  m_eyeOffset = eyeOffset;
  m_position = position;
  m_aabb = core::AxisAlignedBoundingBox(
      glm::vec3(0, 0, 0), glm::vec3(width / 2.0f, height / 2.0f, width / 2.0f));
  m_onGround = false;
  m_flyEnabled = false;
}

Player::~Player() {
  // dtor
}

glm::vec3 &Player::GetVelocity() { return m_velocity; }

glm::vec3 &Player::GetPosition() { return m_position; }

glm::vec3 &Player::GetEyeOffset() { return m_eyeOffset; }

core::SharedPtr<render::ICamera> Player::GetCamera() { return m_cam; }

const core::AxisAlignedBoundingBox &Player::GetAABB() { return m_aabb; }

void Player::Update(float timeStep) {

  if (m_flyEnabled == false) {
    m_velocity += glm::vec3(0, GRAVITY_CONSTANT, 0);

    if (m_velocity.y < -50.0f)
      m_velocity.y = -50;

    if (m_onGround)
      m_velocity.y = 0;

    IsSweptColliding(timeStep);

    if (!m_onGround && IsOnGround()) {
      m_onGround = true;
    } else if (m_onGround && !IsOnGround()) {
      m_onGround = false;
    }
  } else {
    glm::vec3 velocity = m_velocity * timeStep;

    if (velocity.y < TERMINAL_VELOCITY) {
      velocity.y = TERMINAL_VELOCITY;
    }

    m_position = (m_position + velocity);
  }

  m_cam->SetPosition(m_position + m_eyeOffset);
}

void Player::SetFlyEnabled(bool enabled) { m_flyEnabled = enabled; }

bool Player::GetFlyEnabled() { return m_flyEnabled; }

bool Player::Jump(float velocity) {
  if (m_onGround) {
    m_position.y += 0.01;
    m_velocity.y = velocity;
    m_onGround = false;
    //elog::LogInfo(core::string::CFormat("ground %i", 1));
    return true;
  }
  return false;
}

bool Player::OnGround() const { return m_onGround; }

bool Player::IsOnGround() {
  auto pos = this->m_position;
  pos += glm::vec3(0, -0.001f, 0);
  core::AxisAlignedBoundingBox g = m_aabb;
  g.SetCenter(pos);
  return m_octree->CheckCollisionB(g);
}

bool Player::IsColliding() {
  core::AxisAlignedBoundingBox aabb = m_aabb;
  aabb.SetCenter(m_position);
  return m_octree->CheckCollisionB(aabb);
}

bool sortCI(const AABBCollisionInfo &a, const AABBCollisionInfo &b) {
  return a.time < b.time;
}

int GetBlockAxis(const glm::vec3 &n) {
  return (n.x != 0.0f ? 0 : (n.y != 0.0f ? 1 : 2));
}

static bool IsNullVec(const glm::vec3 &n) {
  return (n.x == n.y && n.y == n.z && n.z == 0.0);
}

bool SortCollisions(AABBCollisionInfo &a, AABBCollisionInfo &b) {
  return a.time < b.time;
}

/**
                The algorithm?
1) Get all collision 'points'
2) Sort by collision time
3) Move player according to first collision.
4) Check if still colliding. If so - return to step 1, else we good.. ?
x) Handle precission errors
**/

int MAX_ITERATIONS = 20;
bool Player::IsSweptColliding(float timeStep) {
  /// for decoding voxel coordinates
  glm::vec3 velocity = m_velocity * timeStep;

  for (int i = 0; i < MAX_ITERATIONS; ++i) {
    if (IsNullVec(velocity))
      return false;

    core::AxisAlignedBoundingBox g = m_aabb;
    g.SetCenter(this->m_position);

    auto collisions = m_octree->CheckCollisionSwept(g, velocity);
    std::sort(collisions.begin(), collisions.end(), SortCollisions);

    if (collisions.empty()) {
      m_position += velocity;
      return false;
    }

    glm::vec3 sweepDir = glm::normalize(velocity);
    auto collision = collisions[0];
    float minHitDist = collision.time;
    int axis = GetBlockAxis(collision.normal);
    auto clampedMotion = velocity;
    clampedMotion[axis] *= minHitDist;
    velocity = clampedMotion;
  }
  m_position = m_position + velocity;
  return true;
}

}