#include "game/Game.h"
#include "game/state/AnimationPreviewState.h"

int main() {

  render::SWindowDefinition wDef;
  wDef.Dimensions = {1280, 720};
  wDef.Title = "TheProject2";
  wDef.Fullscreen = false;

  if(game::CGame::Initialize(wDef) == false){
    return -1;
  }

  auto g = std::unique_ptr<game::CGame>(Game);

  auto stateManager = Game->GetGameStateManager();
  stateManager->Register(core::MakeUnique<game::state::AnimationPreviewState>(), game::state::AnimationPreviewState::Name);
  stateManager->Switch(game::state::AnimationPreviewState::Name);

  while (true) {
    Game->GetWindow()->PollEvents();

    auto run = stateManager->Run();

    Game->GetWindow()->SwapBuffers();
    Game->GetRenderer()->EndFrame();

    if(!run){
      break;
    }
  }

  return 0;
}