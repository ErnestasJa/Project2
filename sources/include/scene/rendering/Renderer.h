#ifndef THEPROJECT2_INCLUDE_SCENE_RENDERING_RENDERER_H_
#define THEPROJECT2_INCLUDE_SCENE_RENDERING_RENDERER_H_
#include "render/RenderFwd.h"
#include <object/AnimatedMeshActor.h>

namespace scene {

class Renderer {
public:
  Renderer(render::IRenderer* renderer);

  void Render(game::obj::AnimatedMeshActor * actor);

private:
  render::IRenderer* m_renderer;
};

}

#endif // THEPROJECT2_INCLUDE_SCENE_RENDERING_RENDERER_H_
