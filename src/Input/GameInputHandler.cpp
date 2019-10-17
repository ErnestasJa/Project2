#include "Input/GameInputHandler.h"

namespace input {

GameInputHandler::GameInputHandler() {
  const int maxKeys = 200;
  m_keyStates = core::UniquePtr<int>(new int[maxKeys]);
  for(int i = 0; i < e_input::Keys::Unknown.GetId() + 1; i++){
    m_keyStates.get()[i] = false;
  }
}

bool GameInputHandler::OnKeyDown(const input::Key &key, const bool IsRepeated) {
  m_keyStates.get()[key.GetId()] = true;
  return true;
}

bool GameInputHandler::OnKeyUp(const Key &key, const bool repeated) {
  m_keyStates.get()[key.GetId()] = false;
  return true;
}

bool GameInputHandler::OnMouseMove(const int32_t x, const int32_t y) {
  if(m_mouseMoveHandler){
    m_mouseMoveHandler(x, y);
  }
  return true;
}

void GameInputHandler::SetMouseMoveHandler(
    std::function<void(const int32_t, const int32_t)> mouseMoveHandler) {
  m_mouseMoveHandler = mouseMoveHandler;
}



}