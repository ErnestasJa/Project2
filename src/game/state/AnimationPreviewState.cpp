#include "AnimationPreviewState.h"
#include "../Game.h"
#include "render/AnimatedMesh.h"
#include "render/BaseMaterial.h"

namespace game::state {
const char * AnimationPreviewState::Name = "AnimationPreview";

bool AnimationPreviewState::Initialize() {
  Game->GetWindow()->SetCursorMode(render::CursorMode::HiddenCapture);
  Game->GetRenderer()->SetClearColor(render::Vec3i{155,155,255});
  m_mesh = Game->GetRenderer()->CreateAnimatedMesh();

  auto loader = res::IQMLoader(Game->GetFileSystem());
  loader.Load(m_mesh.get(), io::Path("resources/models/ProjectSteve.iqm"));

  LoadMaterials();

  m_camera = core::MakeShared<render::PerspectiveCamera>(16.0f / 9.0f, 45.0f);
  Game->GetRenderer()->GetRenderContext()->SetCurrentCamera(m_camera);

  m_inputHandlerHandle =
      Game->GetWindow()->GetInputDevice()->AddInputHandler(this);

  m_debugMesh = core::MakeUnique<render::debug::DebugLineMesh>(
      Game->GetRenderer()->CreateBaseMesh(), m_debugMaterial);

  res::mbd::MBDLoader mbdLoader;
  m_bones = mbdLoader.LoadMBD(Game->GetFileSystem(), io::Path("resources/models/ProjectSteve.mbd"));

  m_timer.Start();
  return false;
}

void AnimationPreviewState::LoadMaterials() {
  auto program = Game->GetGpuProgramManager()->LoadProgram(
      "resources/shaders/phong_anim");
  m_material = core::MakeUnique<material::BaseMaterial>(program);

  m_texture = Game->GetImageLoader()->LoadImage(io::Path("resources/models/steve.png"));
  m_material->SetTexture(0, m_texture.get());

  auto debugShader = Game->GetGpuProgramManager()->LoadProgram(
      "resources/shaders/debug");
  m_debugMaterial = core::MakeShared<material::BaseMaterial>(debugShader);
  m_debugMaterial->UseDepthTest = false;
  m_debugMaterial->RenderMode = material::MeshRenderMode::Lines;
}

bool AnimationPreviewState::Finalize() { return false; }

core::String AnimationPreviewState::GetName() {
  return Name;
}

bool AnimationPreviewState::Run() {
  auto delta_ms = m_timer.MilisecondsElapsed();
  float delta_seconds = ((float)delta_ms) / 1000.f;
  m_timer.Start();

  HandleKeyInput(delta_seconds);

  //elog::LogInfo(core::string::format("delta ms: {}", delta_seconds));
  CurrentFrame += 32.0f * delta_seconds;

  Render();

  return m_shouldExitState == false;
}

void AnimationPreviewState::Render(){
  Game->GetRenderer()->BeginFrame();
  Game->GetRenderer()->Clear();

  m_mesh->GetAnimationData().set_interp_frame(CurrentFrame);

  glm::mat4 m(1);
  m = glm::translate(m, glm::vec3(0, 0, 0)) *
      glm::rotate(m, glm::radians(-90.0f), {1.f, 0.f, 0.0f}) *
      glm::scale(m, {1.0f, 1.0f, 1.0f});

  Game->GetRenderer()->RenderMesh(m_mesh.get(), m_material.get(), m);

  const auto &animData = m_mesh->GetAnimationData();
  if(m_bones.empty() == false && m_bones.size() <= animData.bones.size()) {
    m_debugMesh->Clear();
    for (int i = 0; i < m_bones.size(); i++) {
      auto &mbdBone = m_bones[i];

      auto start = glm::vec4(mbdBone.head, 1) * animData.current_frame[i];
      auto end = glm::vec4(mbdBone.tail, 1) * animData.current_frame[i];
      auto color = animData.bone_colors[i];

      m_debugMesh->AddLine(start, end, color);
    }

    m_debugMesh->Upload();

    glm::mat4 m2(1);
    m2 = glm::translate(m2, glm::vec3(0, 0, 0))
       //* glm::rotate(m2, glm::radians(-90.0f), {1.f, 0.f, 0.0f})
       * glm::scale(m, {1.0f, 1.0f, 1.0f});

    Game->GetRenderer()->RenderMesh(m_debugMesh->GetMesh(), m_debugMesh->GetMaterial(), m2);
  }
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