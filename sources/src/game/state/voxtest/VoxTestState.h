#ifndef THEPROJECT2_VOXTEST_H
#define THEPROJECT2_VOXTEST_H
#include "game/state/IGameState.h"
#include <Input/GameInputHandler.h>
#include <render/PerspectiveCamera.h>
#include <render/RenderFwd.h>
#include <util/Timer.h>
#include <voxel/VoxNode.h>

namespace game::state {
class VoxTest : public IGameState, public input::GameInputHandler {
public:
  VoxTest();
  ~VoxTest() override = default;
  bool Initialize() final;
  bool Finalize() final;
  core::String GetName() final;
  bool Run() final;

public:
  void HandleKeyInput(float deltaSeconds);
  bool OnMouseMoveDelta(const int32_t x, const int32_t y) override;
public:
  void GenerateZCurve();
  void DrawGui();

  bool OnKeyUp(const input::Key &key, const bool repeated) override;
private:
  core::SharedPtr<render::PerspectiveCamera> m_camera;
  core::SharedPtr<material::BaseMaterial> m_material;
  bool m_shouldExitState = false;
  input::InputHandlerHandle m_inputHandlerHandle;
  core::SharedPtr<material::BaseMaterial> m_debugMaterial;
  core::UniquePtr<render::debug::DebugLineMesh> m_grid;
  util::Timer m_timer;
  vox::VoxNode m_node;
};
}
#endif // THEPROJECT2_VOXTEST_H
