#include "Input/GameInputHandler.h"

namespace input {

GameInputHandler::GameInputHandler() {
  const int maxKeys = 200;
  m_keyStates = core::UniquePtr<int>(new int[maxKeys]);

  for(int i = 0; i < e_input::Keys::Unknown.GetId() + 1; i++){
    m_keyStates.get()[i] = false;
  }

  m_mouseOld = {0, 0};
  m_mouseNew = {0, 0};
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
    m_mouseOld = m_mouseNew;
    m_mouseNew = {x, y};

    auto delta = m_mouseNew - m_mouseOld;
    OnMouseMoveDelta(delta.x, delta.y);
  return true;
}
bool GameInputHandler::OnMouseMoveDelta(const int32_t x, const int32_t y) {
  return true;
}




}