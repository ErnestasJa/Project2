#ifndef THEPROJECT2_SRC_OBJECT_ANIMATEDMESHACTOR_H_
#define THEPROJECT2_SRC_OBJECT_ANIMATEDMESHACTOR_H_

#include "Actor.h"
#include "render/RenderFwd.h"
#include "render/animation/AnimationFwd.h"

namespace game::obj {

class AnimatedMeshActor: Actor {
public:
  AnimatedMeshActor(core::String name,
                    core::SharedPtr<render::AnimatedMesh> animatedMesh);
  virtual ~AnimatedMeshActor();
  render::AnimatedMesh* GetAnimatedMesh() const;
  void SetMaterial(core::SharedPtr<material::BaseMaterial> material);
  material::BaseMaterial* GetMaterial() const;

  void SetPosition(glm::vec3 position);
  void SetRotation(glm::vec3 rotation);
  void Scale(glm::vec3 scale);

  glm::mat4 GetTransform();

  void Update(float deltaSeconds);
  render::anim::AnimationController* GetAnimationController();

protected:
  core::SharedPtr<render::AnimatedMesh> m_animatedMesh;
  core::UniquePtr<render::anim::AnimationController> m_animationController;
  core::SharedPtr<material::BaseMaterial> m_material;
  glm::vec3 m_position;
  glm::vec3 m_rotation;
  glm::vec3 m_scale;
  glm::mat4 m_transform;
};

}
#endif // THEPROJECT2_SRC_OBJECT_ANIMATEDMESHACTOR_H_
