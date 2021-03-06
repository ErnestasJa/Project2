#include "game/Game.h"
#include "game/state/AnimationPreviewState.h"
#include "game/state/GameState.h"
#include "game/state/voxtest/VoxTestState.h"

int main() {

  render::SWindowDefinition wDef;
  wDef.Dimensions = {1920, 1080};
  wDef.Title = "TheProject2";
  wDef.Fullscreen = false;
  wDef.DebugContext = false;

  if(game::CGame::Initialize(wDef) == false){
    return -1;
  }

  auto g = std::unique_ptr<game::CGame>(Game);

  auto stateManager = Game->GetGameStateManager();
  auto startingState = game::state::GameState::Create();
  //auto startingState = core::MakeUnique<game::state::VoxTest>();
  auto startingStateName = startingState->GetName();

  stateManager->Register(core::Move(startingState), startingStateName);
  stateManager->Switch(startingStateName);

  while (true) {
    Game->GetWindow()->PollEvents();

    auto run = stateManager->Run();

    Game->GetWindow()->SwapBuffers();
    Game->GetRenderer()->EndFrame();

    if(!run){
      auto state = stateManager->GetCurrentState();
      if(state)
      {
        elog::LogInfo("Exiting state: " + state->GetName());
        state->Finalize();
      }
      break;
    }
  }
  g = nullptr;
  return 0;
}