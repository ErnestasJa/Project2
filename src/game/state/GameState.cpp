#include "GameState.h"
#include "game/Game.h"

#include "voxel/VoxelInc.h"
#include "voxel/map/RandomMapGenerator.h"

namespace game::state {
bool GameState::Initialize() {
  Game->GetWindow()->SetCursorMode(render::CursorMode::HiddenCapture);
  Game->GetRenderer()->SetClearColor(render::Vec3i{155, 200, 155});

  m_camera = core::MakeShared<render::OrbitCamera>();
  m_camera->SetPosition({128,150,128});
  m_camera->SetRotation({80,0,0});

  Game->GetRenderer()->GetRenderContext()->SetCurrentCamera(m_camera);
  m_inputHandlerHandle =
      Game->GetWindow()->GetInputDevice()->AddInputHandler(this);

  m_octree = core::MakeShared<vox::MortonOctree>();
  m_meshManager = core::MakeUnique<vox::VoxMeshManager>(Game->GetRenderer(), m_octree);
  m_collisionManager = core::MakeUnique<vox::CollisionManager>(m_octree);

  m_playerActor = core::Move(Game->GetResourceManager()->LoadAssimp(
      "ProjectSteve.fbx", "steve.png", "phong_anim"));
  m_playerActor->SetPosition({128,150,128});

  m_worldMaterial = Game->GetResourceManager()->LoadMaterial("resources/shaders/phong_color");
  vox::map::RandomMapGenerator mapGen({256, 128, 256});

  mapGen.Generate(m_octree.get());
  m_meshManager->GenAllChunks();

  m_player = core::MakeUnique<game::Player>(m_playerActor, m_camera, m_collisionManager.get(), glm::vec3{128,150,128});

  return true;
}

bool GameState::Finalize() { return true; }

core::String GameState::GetName() { return "Game"; }

void GameState::RenderWorld() {
  m_worldMaterial->Use();

  m_worldMaterial->SetMat4("MVP", m_camera->GetProjection() * m_camera->GetView() * glm::mat4(1));

  for (auto chunk : m_meshManager->GetMeshes()) {
    chunk.second->Render();
  }
}

bool GameState::Run() {
  Game->GetRenderer()->BeginFrame();
  Game->GetRenderer()->Clear();

  auto delta = m_timer.SecondsElapsed();
  m_timer.Start();

  m_player->Update(delta);

  HandleKeyInput(delta);
  RenderWorld();
  Game->GetSceneRenderer()->Render(m_playerActor.get());

  return !m_shouldExitState;
}

void GameState::HandleKeyInput(float deltaSeconds) {
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

  if(IsKeyDown(input::Keys::O)){
    m_camera->SetFOV(m_camera->GetFOV() - 1);
    elog::LogInfo(core::string::format("FOV: {}", m_camera->GetFOV()));
  }
  else if(IsKeyDown(input::Keys::P)){
    m_camera->SetFOV(m_camera->GetFOV() + 1);
    elog::LogInfo(core::string::format("FOV: {}", m_camera->GetFOV()));
  }

  bool anyDirectionKeyPressed = wk | ak | sk | dk;

  if (anyDirectionKeyPressed) {
    auto sum = forwardVelocity + strafeVelocity;
    auto direction = glm::normalize(sum);
    auto totalVelocity =
        direction * (supaSpeed ? 10.0f : 5.0f);

    m_player->GetVelocity().x = totalVelocity.x;
    m_player->GetVelocity().z = totalVelocity.z;
  }
  else {
    m_player->GetVelocity().x = 0;
    m_player->GetVelocity().z = 0;
  }

  if(IsKeyDown(input::Keys::Space)){
    m_player->Jump(150);
  }
}

bool GameState::OnMouseMoveDelta(const int32_t x,
                                             const int32_t y) {
  auto rot = m_camera->GetRotation();
  float mouseSpeed = 0.01;
  rot.x -= x * mouseSpeed;
  rot.y -= y * mouseSpeed;

  rot.y = glm::clamp(rot.y, glm::radians(-89.0f), glm::radians(89.0f));

  m_camera->SetRotation(rot);
  return true;
}

core::UniquePtr<IGameState> GameState::Create() {
  return core::MakeUnique<GameState>();
}

}