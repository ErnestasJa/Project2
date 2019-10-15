#ifndef THEPROJECT2_INCLUDE_INPUT_GAMEINPUTHANDLER_H_
#define THEPROJECT2_INCLUDE_INPUT_GAMEINPUTHANDLER_H_
#include "input/InputInc.h"
#include <functional>

namespace e_input = input;
namespace input {
class GameInputHandler: public e_input::InputHandler {
public:
  GameInputHandler();
  bool OnKeyDown(const input::Key &key,
                         const bool IsRepeated) override;
  bool OnMouseMove(const int32_t x, const int32_t y) override;
  bool OnKeyUp(const Key& key, const bool repeated) override;

  bool IsKeyDown(const Key& key) const {
    return m_keyStates.get()[key.GetId()];
  }

  void SetMouseMoveHandler(std::function<void(const int32_t, const int32_t)> mouseMoveHandler);
private:
  core::UniquePtr<int> m_keyStates;
  std::function<void(const int32_t, const int32_t)> m_mouseMoveHandler;
};
}
#endif // THEPROJECT2_INCLUDE_INPUT_GAMEINPUTHANDLER_H_
