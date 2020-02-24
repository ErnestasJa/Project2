#include "AnimationPreviewState.h"
#include "../Game.h"
#include "render/AnimatedMesh.h"
#include "render/BaseMaterial.h"
#include "resource_management/mesh/AssimpImport.h"
#include "object/AnimatedMeshActor.h"
#include "render/animation/AnimationController.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace game::state {

bool AnimationPreviewState::Initialize() {
  Game->GetWindow()->SetCursorMode(render::CursorMode::HiddenCapture);
  Game->GetRenderer()->SetClearColor(render::Vec3i{155,155,255});

  LoadMaterials();

  m_camera = core::MakeShared<render::PerspectiveCamera>(16.0f / 9.0f, 45.0f);
  Game->GetRenderer()->GetRenderContext()->SetCurrentCamera(m_camera);

  m_inputHandlerHandle =
      Game->GetWindow()->GetInputDevice()->AddInputHandler(this);

  auto gridMat = m_debugMaterial->Instance();
  gridMat->UseDepthTest = true;
  m_grid = core::MakeUnique<render::debug::DebugLineMesh>(
      Game->GetRenderer()->CreateBaseMesh(), gridMat);

  m_grid->AddGrid(50, 1, glm::tvec3<uint8_t>(0,0,0));
  m_grid->Upload();

  m_playerActor = Game->GetResourceManager()->LoadAssimp("ProjectSteve.fbx", "steve.png", "phong_anim");
  m_weaponActor = Game->GetResourceManager()->LoadAssimp("pickaxe.fbx", "mc_gear.png", "phong");
  m_weaponActor->SetPosition({0,0,1});

  if(!m_playerActor || !m_weaponActor){
    m_shouldExitState = true;
  }

  m_timer.Start();

  return false;
}

void AnimationPreviewState::LoadMaterials() {
  m_debugMaterial = Game->GetResourceManager()->LoadMaterial("resources/shaders/debug");
  m_debugMaterial->UseDepthTest = false;
  m_debugMaterial->RenderMode = material::MeshRenderMode::Lines;
}

bool AnimationPreviewState::Finalize() {
  return false;
}

core::String AnimationPreviewState::GetName() {
  return "AnimationPreview";
}

bool AnimationPreviewState::Run() {
  if(m_shouldExitState){
    return false;
  }

  auto delta_ms = m_timer.MilisecondsElapsed();
  float delta_seconds = ((float)delta_ms) / 1000.f;
  m_timer.Start();

  HandleKeyInput(delta_seconds);

  //elog::LogInfo(core::string::format("delta ms: {}", delta_seconds));
  m_playerActor->Update(delta_seconds);
  Render();

  return true;
}

void AnimationPreviewState::Render(){
  Game->GetRenderer()->BeginFrame();
  Game->GetRenderer()->Clear();

  Game->GetRenderer()->RenderMesh(m_grid->GetMesh(), m_grid->GetMaterial(), glm::mat4(1));
  Game->GetSceneRenderer()->Render(m_playerActor.get());

  auto weaponSlotTransform = m_playerActor->GetAnimationController()->GetBoneTransformation("weapon");
  auto weaponTransform = m_weaponActor->GetArmature().GetBones()[0].offset;

  auto weapTransform = (weaponSlotTransform * weaponTransform);
  glm::vec3 pos, scale, skew;
  glm::quat rot;
  glm::vec4 perspective;

  glm::decompose(weapTransform, scale, rot, pos, skew, perspective);

  m_weaponActor->SetPosition(pos);
  m_weaponActor->SetRotation(glm::eulerAngles(rot));

  Game->GetSceneRenderer()->Render(m_weaponActor.get());
}

void AnimationPreviewState::HandleKeyInput(float deltaSeconds) {
  m_shouldExitState |= IsKeyDown(input::Keys::Esc);

  auto look = m_camera->GetLocalZ();
  auto right = m_camera->GetLocalX();

  look = glm::normalize(look);
  right = glm::normalize(right);

  auto wk = IsKeyDown(input::Keys::W);
  auto sk = IsKeyDown(input::Keys::S);
  auto dk = IsKeyDown(input::Keys::D);
  auto ak = IsKeyDown(input::Keys::A);
  auto supaSpeed = IsKeyDown(input::Keys::X);

  glm::vec3 forwardVelocity(0), strafeVelocity(0);

  if (wk) {
    forwardVelocity.x = look.x;
    forwardVelocity.y = look.y;
    forwardVelocity.z = look.z;
  } else if (sk) {
    forwardVelocity.x = -look.x;
    forwardVelocity.y = -look.y;
    forwardVelocity.z = -look.z;
  }

  if (dk) {
    strafeVelocity.x = right.x;
    strafeVelocity.y = right.y;
    strafeVelocity.z = right.z;
  } else if (ak) {
    strafeVelocity.x = -right.x;
    strafeVelocity.y = -right.y;
    strafeVelocity.z = -right.z;
  }

  if(IsKeyDown(input::Keys::F)){
    m_playerActor->GetAnimationController()->SetAnimation("Armature|attack");
  }
  else if(IsKeyDown(input::Keys::G)){
    m_playerActor->GetAnimationController()->SetAnimation("Armature|walk");
  }

  bool anyDirectionKeyPressed = wk | ak | sk | dk;

  if (anyDirectionKeyPressed) {
    auto sum = forwardVelocity + strafeVelocity;
    auto direction = glm::normalize(sum);
    auto totalVelocity =
        direction * (supaSpeed ? CameraSpeed * 10 : CameraSpeed);

    auto currentPos = m_camera->GetPosition();
    currentPos += totalVelocity;
    m_camera->SetPosition(currentPos);
  }
}

bool AnimationPreviewState::OnMouseMoveDelta(const int32_t x,
                                             const int32_t y) {
  auto rot = m_camera->GetRotation();
  float mouseSpeed = 0.01;
  rot.x -= x * mouseSpeed;
  rot.y -= y * mouseSpeed;

  rot.y = glm::clamp(rot.y, glm::radians(-89.0f), glm::radians(89.0f));

  m_camera->SetRotation(rot);
  return true;
}

}