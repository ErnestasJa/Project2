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
  virtual bool OnMouseMoveDelta(const int32_t x, const int32_t y);
  bool OnKeyUp(const Key& key, const bool repeated) override;

  bool IsKeyDown(const Key& key) const {
    return m_keyStates.get()[key.GetId()];
  }

private:
    std::function<void(const int32_t, const int32_t)> m_mouseMoveHandler;
    core::pod::Vec2<int32_t> m_mouseOld = {0, 0};
    core::pod::Vec2<int32_t> m_mouseNew = {0, 0};
    core::UniquePtr<int> m_keyStates;
};
}
#endif // THEPROJECT2_INCLUDE_INPUT_GAMEINPUTHANDLER_H_
