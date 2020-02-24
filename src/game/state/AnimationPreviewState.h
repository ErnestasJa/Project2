#ifndef ANIMATIONPREVIEWSTATE_H
#define ANIMATIONPREVIEWSTATE_H

#include "IGameState.h"
#include "resource_management/mesh/MBDBone.h"
#include <Input/GameInputHandler.h>
#include <game/Player.h>
#include <object/AnimatedMeshActor.h>
#include <render/PerspectiveCamera.h>
#include <render/RenderFwd.h>
#include <resource_management/mesh/AssimpImport.h>
#include <util/Timer.h>

namespace game::state {
class AnimationPreviewState : public IGameState, public input::GameInputHandler {
public:
  float CameraSpeed = 0.2f;
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
  util::Timer m_timer;
  input::InputHandlerHandle m_inputHandlerHandle;
  core::SharedPtr<render::PerspectiveCamera> m_camera;
  core::SharedPtr<material::BaseMaterial> m_debugMaterial;
  core::UniquePtr<render::debug::DebugLineMesh> m_grid;
  core::UniquePtr<game::obj::AnimatedMeshActor> m_playerActor;
  core::UniquePtr<game::obj::AnimatedMeshActor> m_weaponActor;
  bool m_shouldExitState = false;
};
} // namespace game::state

#endif