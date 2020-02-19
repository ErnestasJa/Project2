#include "object/AnimatedMeshActor.h"
#include "render/animation/AnimationController.h"
#include "render/AnimatedMesh.h"
#include <glm/gtx/euler_angles.hpp>

namespace game::obj {
AnimatedMeshActor::AnimatedMeshActor(core::String name,
    core::SharedPtr<render::AnimatedMesh> animatedMesh)
    : Actor(name), m_position(0,0,0), m_rotation(0,0,0), m_scale(1,1,1) {
  m_animatedMesh = animatedMesh;

  if(m_animatedMesh->GetAnimations().size()) {
    m_animationController =
        core::MakeUnique<render::anim::AnimationController>(animatedMesh.get());
    m_animationController->SetAnimation(0);
    m_animationController->Animate(0);
  }
}

AnimatedMeshActor::~AnimatedMeshActor(){}

render::AnimatedMesh *AnimatedMeshActor::GetAnimatedMesh() const {
  return m_animatedMesh.get();
}

void AnimatedMeshActor::SetMaterial(
    core::SharedPtr<material::BaseMaterial> material) {
  m_material = material;
}

material::BaseMaterial *AnimatedMeshActor::GetMaterial() const {
  return m_material.get();
}

void AnimatedMeshActor::SetPosition(glm::vec3 position) {
  m_position = position;
}

void AnimatedMeshActor::SetRotation(glm::vec3 rotation) {
  m_rotation = rotation;
}

void AnimatedMeshActor::Scale(glm::vec3 scale) {
  m_scale = scale;
}

void AnimatedMeshActor::Update(float deltaSeconds) {
  m_animationController->Animate(deltaSeconds);
}


glm::mat4 AnimatedMeshActor::GetTransform() {
  return glm::translate(glm::mat4(1), m_position) /** glm::yawPitchRoll(m_rotation.x, m_rotation.y, m_rotation.z) * glm::scale(glm::mat4(1), m_scale)*/;
}

render::anim::AnimationController *AnimatedMeshActor::GetAnimationController() {
  return m_animationController.get();
}

}