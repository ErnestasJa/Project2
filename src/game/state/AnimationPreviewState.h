#ifndef ANIMATIONPREVIEWSTATE_H
#define ANIMATIONPREVIEWSTATE_H

#include "IGameState.h"
#include <Input/GameInputHandler.h>
#include <game/Player.h>
#include <render/PerspectiveCamera.h>
#include <render/RenderFwd.h>
#include <util/Timer.h>
#include "resource_management/mesh/MBDBone.h"

namespace game::state {
class AnimationPreviewState : public IGameState, public input::GameInputHandler {
public:
  float CameraSpeed = 0.2f;
  float CurrentFrame = 0;
  static const char * Name;
public:
  bool Initialize() final;
  bool Finalize() final;
  core::String GetName() final;
  bool Run() final;

public:
  void HandleKeyInput(float deltaSeconds);
  bool OnMouseMoveDelta(const int32_t x, const int32_t y) override;
  void LoadMaterials();
  void Render();

private:
  core::UniquePtr<render::AnimatedMesh> m_mesh;
  core::UniquePtr<material::BaseMaterial> m_material;
  util::Timer m_timer;
  core::SharedPtr<render::PerspectiveCamera> m_camera;
  input::InputHandlerHandle m_inputHandlerHandle;
  core::SharedPtr<material::BaseMaterial> m_debugMaterial;
  core::Vector<res::mbd::Bone> m_bones;
  core::UniquePtr<render::debug::DebugLineMesh> m_debugMesh;
  bool m_shouldExitState = false;
  core::SharedPtr<render::ITexture> m_texture;
};
} // namespace game::state

#endif