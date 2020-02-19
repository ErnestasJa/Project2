#ifndef THEPROJECT2_SRC_OBJECT_ACTOR_H_
#define THEPROJECT2_SRC_OBJECT_ACTOR_H_

namespace game::obj {
class Actor {
public:
  Actor(core::String name);
  virtual ~Actor();

  core::String GetName() const;

protected:
  core::String m_name;
};
}
#endif // THEPROJECT2_SRC_OBJECT_ACTOR_H_
