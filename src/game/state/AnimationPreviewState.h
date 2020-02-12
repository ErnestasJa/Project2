#ifndef ANIMATIONPREVIEWSTATE_H
#define ANIMATIONPREVIEWSTATE_H

#include "IGameState.h"
#include "resource_management/mesh/MBDBone.h"
#include <Input/GameInputHandler.h>
#include <game/Player.h>
#include <render/PerspectiveCamera.h>
#include <render/RenderFwd.h>
#include <resource_management/mesh/AssimpImport.h>
#include <util/Timer.h>

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
  void LoadAnimatedMesh(core::String name);

private:
  core::UniquePtr<render::AnimatedMesh> m_mesh;
  core::UniquePtr<material::BaseMaterial> m_animMeshMaterial;
  util::Timer m_timer;
  core::SharedPtr<render::PerspectiveCamera> m_camera;
  input::InputHandlerHandle m_inputHandlerHandle;
  core::SharedPtr<material::BaseMaterial> m_debugMaterial;
  core::Vector<res::mbd::Bone> m_bones;
  core::UniquePtr<render::debug::DebugLineMesh> m_debugMesh;
  core::UniquePtr<render::debug::DebugLineMesh> m_grid;
  bool m_shouldExitState = false;
  core::SharedPtr<render::ITexture> m_texture;
  core::UniquePtr<res::mesh::AssimpImport> m_assimpImporter;
  core::UniquePtr<render::AnimatedMesh> m_steve;
  core::SharedPtr<material::BaseMaterial> m_phongMaterial;
  void RenderBones(float time);
};
} // namespace game::state

#endif