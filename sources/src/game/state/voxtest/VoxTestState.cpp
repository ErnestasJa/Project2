//
// Created by serengeor on 10/11/20.
//
#include "VoxTestState.h"
#include <game/Game.h>
#include <imgui.h>
#include <util/thread/Sleep.h>
#include <voxel/VoxelInc.h>
#include "gui/IGui.h"

namespace game::state {
VoxTest::VoxTest() {

}

bool VoxTest::Initialize() {
  Game->GetWindow()->SetCursorMode(render::CursorMode::HiddenCapture);
  Game->GetRenderer()->SetClearColor(render::Vec3i{155,155,255});

  m_inputHandlerHandle =
      Game->GetWindow()->GetInputDevice()->AddInputHandler(this);
  m_debugMaterial = Game->GetResourceManager()->LoadMaterial("resources/shaders/debug");
  m_grid = core::MakeUnique<render::debug::DebugLineMesh>(
      Game->GetRenderer()->CreateBaseMesh(), m_debugMaterial);

  m_camera = core::MakeShared<render::PerspectiveCamera>(16.0f / 9.0f, 45.0f);
  Game->GetRenderer()->GetRenderContext()->SetCurrentCamera(m_camera);

  m_timer.Start();
  m_node = vox::VoxNode(0,0,0);
  m_node.size = 4;

  GenerateZCurve();

  return false;
}

bool VoxTest::Finalize() {

  return false;
}

core::String VoxTest::GetName() {
  return "VoxTest";
}

void VoxTest::GenerateZCurve() {
  m_grid->Clear();

  m_grid->AddGrid(100, 1, {255, 0, 0}, {0,-1,0});

  if(m_node.size < 2){
    return;
  }
  auto end = m_node.start + m_node.size-1;

  for(int32_t i = m_node.start; i <= end; i++){
    auto [x,y,z] = vox::utils::Decode(i);
    auto [x2,y2,z2] = vox::utils::Decode(i+1);

    m_grid->AddLine({x,y,z}, {x2, y2, z2}, {0, 0,255});
  }

  m_grid->Upload();
}

void VoxTest::DrawGui() {
  Game->GetGui()->BeginRender();

  ImGui::Begin("Z-Curve settings");

  int32_t start = m_node.start, size = m_node.size;

  int32_t maxSize = 128 * 128 * 128;

  auto changed = ImGui::DragInt("Start", &start, 1, 0, maxSize) |
      ImGui::DragInt("Size", &size, 1, 2, maxSize - 2);

  if(changed){
    m_node.start = start;
    m_node.size = size;
    GenerateZCurve();
  }

  ImGui::End();
  Game->GetGui()->EndRender();
}

bool VoxTest::Run() {
  if(m_shouldExitState){
    return false;
  }
  util::Timer timer;


  auto delta_seconds = ((float)m_timer.MilisecondsElapsed()) / 1000.f;
  m_timer.Start();
  HandleKeyInput(delta_seconds);

  Game->GetRenderer()->BeginFrame();
  Game->GetRenderer()->Clear();
  DrawGui();

  Game->GetRenderer()->RenderMesh(m_grid->GetMesh(), m_grid->GetMaterial(), glm::mat4(1));

  //------
  int32_t frameSleepMicroSeconds = 10000 - timer.MicrosecondsElapsed();

  if (frameSleepMicroSeconds > 0) {
    util::thread::Sleep(frameSleepMicroSeconds);
  }

  return true;
}



void VoxTest::HandleKeyInput(float deltaSeconds) {
  auto look = glm::normalize(m_camera->GetLocalZ());
  auto right = glm::normalize(m_camera->GetLocalX());

  auto wk = IsKeyDown(input::Keys::W);
  auto sk = IsKeyDown(input::Keys::S);
  auto dk = IsKeyDown(input::Keys::D);
  auto ak = IsKeyDown(input::Keys::A);
  auto speed = (IsKeyDown(input::Keys::L_SHIFT) ? 100.0f : 5.0f) * deltaSeconds;

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
    auto totalVelocity = direction * speed;

    m_camera->SetPosition(m_camera->GetPosition() + totalVelocity);
  }

  if(IsKeyDown(input::Keys::SPACE)) {
    auto pos = m_camera->GetPosition();
    pos.y += speed;
    m_camera->SetPosition(pos);
  }
  else if(IsKeyDown(input::Keys::L_CTRL)){
    auto pos = m_camera->GetPosition();
    pos.y -= speed;
    m_camera->SetPosition(pos);
  }
}

bool VoxTest::OnKeyUp(const input::Key &key, const bool repeated) {
  if (key == input::Keys::GRAVE_ACCENT) {
    auto cursorMode =
        Game->GetWindow()->GetCursorMode() == render::CursorMode::Normal
        ? render::CursorMode::HiddenCapture
        : render::CursorMode::Normal;

    Game->GetWindow()->SetCursorMode(cursorMode);
  }
  else if (key == input::Keys::ESC) {
    m_shouldExitState = true;
  }

  return GameInputHandler::OnKeyUp(key, repeated);
}

bool VoxTest::OnMouseMoveDelta(const int32_t x, const int32_t y) {
  auto cursorMode =
      Game->GetWindow()->GetCursorMode() == render::CursorMode::Normal
      ? render::CursorMode::HiddenCapture
      : render::CursorMode::Normal;

  if(cursorMode != render::CursorMode::Normal){
    return true;
  }

  auto rot = m_camera->GetRotation();
  float mouseSpeed = 0.01;
  rot.x -= x * mouseSpeed;
  rot.y -= y * mouseSpeed;

  rot.y = glm::clamp(rot.y, glm::radians(-89.0f), glm::radians(89.0f));

  m_camera->SetRotation(rot);
  return true;
}

}
