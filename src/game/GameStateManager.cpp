#include "GameStateManager.h"

namespace game {

GameStateManager::GameStateManager()
    : m_currentState(nullptr), m_nextState(nullptr) {}

GameStateManager::~GameStateManager() {
  if(m_currentState){
    m_currentState->Finalize();
  }
}

void GameStateManager::Register(core::UniquePtr<game::state::IGameState> state,
                                core::String name) {
  m_stateMap[name] = std::move(state);
}

void GameStateManager::Switch(core::String name) {
  auto stateIt = m_stateMap.find(name);

  if (stateIt == m_stateMap.end()) {
    elog::LogError("Couldn't find game state: " + name);
    return;
  }

  m_nextState = (*stateIt).second.get();
}

bool GameStateManager::Run() {
  if (m_nextState) {
    if (m_currentState) {
      m_currentState->Finalize();
    }

    m_currentState = m_nextState;
    m_currentState->Initialize();
    m_nextState = nullptr;
  }

  bool result = false;
  if (m_currentState) {
    result = m_currentState->Run();
  }
  return result;
}
}