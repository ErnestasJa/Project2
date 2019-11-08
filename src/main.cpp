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

  auto stateManager = Game->GetGameStateManager();
  stateManager->Register(core::MakeUnique<game::state::AnimationPreviewState>(), game::state::AnimationPreviewState::Name);
  stateManager->Switch(game::state::AnimationPreviewState::Name);

  while (stateManager->Run()) {

  }

  return 0;
}