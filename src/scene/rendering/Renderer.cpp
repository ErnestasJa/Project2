#include "scene/rendering/Renderer.h"
#include "render/IRenderer.h"
#include "render/ICamera.h"
#include "render/IRenderContext.h"
#include "render/AnimatedMesh.h"
#include "render/BaseMaterial.h"
#include "render/animation/AnimationController.h"

namespace scene {
Renderer::Renderer(render::IRenderer* renderer): m_renderer(renderer)
{

}

void Renderer::Render(game::obj::AnimatedMeshActor *actor) {
  auto mesh = actor->GetAnimatedMesh();
  auto material = actor->GetMaterial();
  auto cam = m_renderer->GetRenderContext()->GetCurrentCamera();

  m_renderer->GetRenderContext()->SetDepthTest(material->UseDepthTest);

  material->Use();
  material->SetMat4("MVP", cam->GetProjection() * cam->GetView() * actor->GetTransform());

  if(auto animCtl = actor->GetAnimationController(); animCtl){
    material->SetMat4("Bones", animCtl->GetCurrentFrame().data(), animCtl->GetCurrentFrame().size(), true);
  }

  m_renderer->SetActiveTextures(material->GetTextures());
  mesh->Render();
}
}