#ifndef THEPROJECT2_GAMESTATEMANAGER_H
#define THEPROJECT2_GAMESTATEMANAGER_H

#include "game/state/IGameState.h"

namespace game {
    class GameStateManager {
    public:
        GameStateManager();
        ~GameStateManager();
        void Register(core::UniquePtr<state::IGameState> state, core::String name);
        void Switch(core::String name);
        bool Run();

        state::IGameState* GetCurrentState(){
          return m_currentState;
        }

    private:
        core::UnorderedMap<core::String, core::UniquePtr<state::IGameState>> m_stateMap;
        state::IGameState* m_currentState;
        state::IGameState* m_nextState;
    };
}

#endif //THEPROJECT2_GAMESTATEMANAGER_H
