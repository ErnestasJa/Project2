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

  virtual bool Initialize() override;
  virtual bool Finalize() override;
  virtual core::String GetName() override;
  virtual bool Run() override;

protected:
  void RenderWorld();
  void HandleKeyInput(float deltaSeconds);
  bool OnMouseMoveDelta(const int32_t x, const int32_t y) override;

protected:
  util::Timer m_timer;
  input::InputHandlerHandle m_inputHandlerHandle;
  core::SharedPtr<render::OrbitCamera> m_camera;
  core::SharedPtr<game::obj::AnimatedMeshActor> m_playerActor;
  core::SharedPtr<vox::MortonOctree> m_octree;
  core::UniquePtr<vox::VoxMeshManager> m_meshManager;
  core::UniquePtr<vox::CollisionManager> m_collisionManager;
  core::SharedPtr<material::BaseMaterial> m_worldMaterial;
  core::UniquePtr<game::Player> m_player;
  bool m_shouldExitState = false;
  core::UniquePtr<render::ITexture> m_worldAtlas;
};


}
#endif // THEPROJECT2_SRC_GAME_STATE_GAMESTATE_H_
