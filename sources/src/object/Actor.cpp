#include "object/Actor.h"

namespace game::obj {
Actor::Actor(core::String name)
: m_name(name){
}

Actor::~Actor(){};

core::String Actor::GetName() const{
  return m_name;
}
}