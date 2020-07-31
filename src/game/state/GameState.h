#ifndef THEPROJECT2_SRC_GAME_STATE_GAMESTATE_H_
#define THEPROJECT2_SRC_GAME_STATE_GAMESTATE_H_

#include "IGameState.h"
#include "game/GameFwd.h"
#include "render/RenderFwd.h"
#include "voxel/VoxelFwd.h"
#include <Input/GameInputHandler.h>
#include <game/Player.h>
#include <input/InputHandlerHandle.h>
#include <util/Timer.h>

namespace game::state {
class GameState: public IGameState, public input::GameInputHandler {
public:
  static core::UniquePtr<IGameState> Create();
  ~GameState() override;

  bool Initialize() override;
  bool Finalize() override;
  core::String GetName() override;
  bool Run() override;

protected:
  void RenderWorld();
  void RenderPlayer(float deltaSeconds);
  void RenderGui(float deltaSeconds);
  void HandleKeyInput(float deltaSeconds);
  bool OnMouseMoveDelta(const int32_t x, const int32_t y) override;
  bool OnMouseDown(const input::MouseButton& key) override;
  bool OnMouseUp(const input::MouseButton& key) override;
  bool OnKeyUp(const input::Key& key, const bool repeated) override;

  void GenerateNoiseImage();

protected:
  util::Timer m_timer;
  input::InputHandlerHandle m_inputHandlerHandle;
  core::SharedPtr<render::OrbitCamera> m_camera;
  core::SharedPtr<game::obj::AnimatedMeshActor> m_playerActor;
  core::SharedPtr<game::obj::AnimatedMeshActor> m_weaponActor;
  core::SharedPtr<vox::MortonOctree> m_octree;
  core::UniquePtr<vox::VoxMeshManager> m_meshManager;
  core::UniquePtr<vox::CollisionManager> m_collisionManager;
  core::SharedPtr<material::BaseMaterial> m_worldMaterial;
  core::UniquePtr<render::DebugRenderer> m_debugRenderer;
  core::UniquePtr<game::Player> m_player;
  bool m_shouldExitState = false;
  core::UniquePtr<render::ITexture> m_worldAtlas;
  core::tuple<glm::vec3, glm::vec3> GetPlayerAimDirection();

  core::UniquePtr<render::ITexture> m_noiseTexture;
  core::UniquePtr<render::Image> m_noiseImage;
};


}
#endif // THEPROJECT2_SRC_GAME_STATE_GAMESTATE_H_
