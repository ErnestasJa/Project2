#include "GameState.h"
#include "game/Game.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "util/Noise.h"
#include "util/noise/NoiseGenerator.h"

#include "gui/IGui.h"
#include "render/Image.h"
#include "render/animation/AnimationController.h"
#include "render/debug/DebugRenderer.h"
#include "util/thread/Sleep.h"
#include "voxel/VoxelInc.h"
#include "voxel/map/RandomMapGenerator.h"

namespace game::state {
static core::pod::Vec3 G_WorldSize(32, 16, 32);

GameState::~GameState() { m_inputHandlerHandle.Disconnect(); }

bool GameState::Initialize() {
  Game->GetWindow()->SetCursorMode(render::CursorMode::Normal);
  Game->GetRenderer()->SetClearColor(render::Vec3i{155, 200, 155});

  m_camera = core::MakeShared<render::OrbitCamera>();
  m_camera->SetPosition({128, 150, 128});
  m_camera->SetRotation({80, 0, 0});
  m_camera->SetDistance(6);
  m_camera->SetFOV(35);

  Game->GetRenderer()->GetRenderContext()->SetCurrentCamera(m_camera);
  m_inputHandlerHandle =
      Game->GetWindow()->GetInputDevice()->AddInputHandler(this);

  m_octree = core::MakeShared<vox::MortonOctree>();
  m_meshManager =
      core::MakeUnique<vox::VoxMeshManager>(Game->GetRenderer(), m_octree);
  m_collisionManager = core::MakeUnique<vox::CollisionManager>(m_octree);

  m_playerActor = core::Move(Game->GetResourceManager()->LoadAssimp(
      "ProjectSteve.fbx", "steve.png", "phong_anim"));
  m_weaponActor = Game->GetResourceManager()->LoadAssimp(
      "pickaxe.fbx", "mc_gear.png", "phong");

  m_worldAtlas = Game->GetImageLoader()->LoadAtlasAs2DTexture(
      io::Path("resources/textures/block_atlas.png"), 48);

  m_worldMaterial =
      Game->GetResourceManager()->LoadMaterial("resources/shaders/voxel");
  m_worldMaterial->SetTexture(0, m_worldAtlas.get());
  vox::map::RandomMapGenerator mapGen(
      {G_WorldSize.x, G_WorldSize.y, G_WorldSize.z});

  mapGen.Generate(m_octree.get());
  m_meshManager->GenAllChunks();

  m_debugRenderer = core::MakeUnique<render::DebugRenderer>(
      460, Game->GetRenderer(), Game->GetResourceManager());

  m_player = core::MakeUnique<game::Player>(
      m_debugRenderer.get(), m_playerActor, m_camera, m_collisionManager.get(),
      glm::vec3{G_WorldSize.x / 2, G_WorldSize.y, G_WorldSize.z / 2});

  m_noiseImage = core::MakeUnique<render::Image>(
      core::pod::Vec2<uint32_t>{512, 512}, render::ImageFormat::RGB);
  m_noiseTexture = Game->GetRenderer()->CreateTexture(render::TextureDescriptor(
      m_noiseImage->GetSize().x, m_noiseImage->GetSize().y, render::TextureDataFormat::RGB));
  m_noiseGenerator = core::MakeUnique<util::noise::NoiseGenerator>(core::pod::Vec3<int32_t>(m_noiseImage->GetSize().x, m_noiseImage->GetSize().y,1));

  GenerateNoiseImage();
  return true;
}

bool GameState::Finalize() { return true; }

core::String GameState::GetName() { return "Game"; }

void GameState::GenerateNoiseImage() {
  auto size = m_noiseImage->GetSize();
  //siv::PerlinNoise n(m_timer.SecondsSinceEpoch());

  m_noiseGenerator->GenSimplex();
  for (uint32_t x = 0; x < size.x; x++) {
    for (uint32_t y = 0; y < size.y; y++) {
      uint32_t value = m_noiseGenerator->GetNoise(x,y,0);
      m_noiseImage->WritePixel(x, y, value, value, value);
    }
  }

  m_noiseTexture->UploadData(render::TextureDataDescriptor(
      (void*)m_noiseImage->GetData(), m_noiseImage->GetSize()));
}

util::noise::NoiseGeneratorSettings noiseSettings;
bool settingsChanged = true;
int32_t seed = 1234;

void GameState::RenderGui(float deltaSeconds) {
  Game->GetGui()->BeginRender();

  ImGui::Begin("Player settings");

  bool fly = m_player->GetFlyEnabled();
  if(ImGui::Checkbox("Fly", &fly)){
    m_player->SetFlyEnabled(fly);
  }

  ImGui::End();

  ImGui::Begin("Noise gen");

  bool seedChanged = ImGui::SliderInt("Seed", &seed, 0, 9999);
  if(seedChanged){
    m_noiseGenerator->Reseed(m_timer.SecondsSinceEpoch());
  }

  settingsChanged =
  ImGui::SliderFloat("Min val", &noiseSettings.Min, 0, 1) |
  ImGui::SliderFloat("Max val", &noiseSettings.Max, 0, 1) |
  ImGui::SliderFloat("Offset", &noiseSettings.Offset, -1, 1) |
  ImGui::SliderFloat("Fractal gain", &noiseSettings.FractalGain, 0, 1) |
  ImGui::SliderFloat("Fractal lacunarity", &noiseSettings.FractalLacunarity, 0, 5) |
  ImGui::SliderInt("Fractal octaves", &noiseSettings.FractalOctaves, 1, 15) |
  ImGui::SliderFloat("Frequency", &noiseSettings.Frequency, 0.0001, 0.1);

  if(ImGui::Button("Reset")){
    noiseSettings = util::noise::NoiseGeneratorSettings();
    settingsChanged = true;
  }


  m_noiseGenerator->SetNoiseGenSettings(noiseSettings);

  if (settingsChanged || seedChanged) {
    GenerateNoiseImage();
  }

  auto tId = m_noiseTexture->GetId();

  ImGui::Image((void *)tId, ImVec2(m_noiseImage->GetSize().x, m_noiseImage->GetSize().y));

  ImGui::End();

  Game->GetGui()->EndRender();
}

void GameState::RenderWorld() {
  m_worldMaterial->Use();
  m_worldMaterial->SetMat4("MVP", m_camera->GetProjection() *
                                      m_camera->GetView() * glm::mat4(1));

  Game->GetRenderer()->GetRenderContext()->SetDepthTest(
      m_worldMaterial->UseDepthTest);
  Game->GetRenderer()->SetActiveTextures(m_worldMaterial->GetTextures());

  for (auto chunk : m_meshManager->GetMeshes()) {
    chunk.second->Render();
  }
}

void GameState::RenderPlayer(float deltaSeconds) {
  auto weaponSlotTransform =
      m_playerActor->GetAnimationController()->GetBoneTransformation("weapon");
  auto weaponTransform = m_weaponActor->GetArmature().GetBones()[0].offset;

  auto weapTransform =
      (m_playerActor->GetTransform() * weaponSlotTransform * weaponTransform);
  glm::vec3 pos, scale, skew;
  glm::quat rot;
  glm::vec4 perspective;

  glm::decompose(weapTransform, scale, rot, pos, skew, perspective);

  m_weaponActor->SetPosition(pos);
  m_weaponActor->SetRotation(glm::eulerAngles(rot));

  Game->GetSceneRenderer()->Render(m_weaponActor.get());
  Game->GetSceneRenderer()->Render(m_playerActor.get());
}

bool GameState::Run() {
  util::Timer timer;
  Game->GetRenderer()->BeginFrame();
  Game->GetRenderer()->Clear();

  auto secondsElapsed = m_timer.SecondsElapsed();
  auto milisecondsElapsed = m_timer.MilisecondsElapsed();
  m_timer.Start();

  m_debugRenderer->Update(milisecondsElapsed);

  m_player->Update(secondsElapsed);

  HandleKeyInput(secondsElapsed);
  RenderWorld();
  RenderPlayer(secondsElapsed);
  m_debugRenderer->Render();
  RenderGui(secondsElapsed);

  if (Game->GetRenderer()->GetDebugMessageMonitor()->isDebuggingEnabled()) {
    for (auto msg :
         Game->GetRenderer()->GetDebugMessageMonitor()->GetMessages()) {
      elog::LogInfo(msg->AsString());
    }
    Game->GetRenderer()->GetDebugMessageMonitor()->ClearMessages();
  }

  int32_t frameSleepMicroSeconds = 10000 - timer.MicrosecondsElapsed();

  if (frameSleepMicroSeconds > 0) {
    util::thread::Sleep(frameSleepMicroSeconds);
  }

  return !m_shouldExitState;
}

void GameState::HandleKeyInput(float deltaSeconds) {
  m_shouldExitState |= IsKeyDown(input::Keys::ESC);

  auto look = m_camera->GetLocalZ();
  auto right = m_camera->GetLocalX();

  look = glm::normalize(look);
  right = glm::normalize(right);

  auto wk = IsKeyDown(input::Keys::W);
  auto sk = IsKeyDown(input::Keys::S);
  auto dk = IsKeyDown(input::Keys::D);
  auto ak = IsKeyDown(input::Keys::A);
  auto supaSpeed = IsKeyDown(input::Keys::L_SHIFT);

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

  if (IsKeyDown(input::Keys::O)) {
    m_camera->SetFOV(m_camera->GetFOV() - 1);
    elog::LogInfo(core::string::format("FOV: {}", m_camera->GetFOV()));
  } else if (IsKeyDown(input::Keys::P)) {
    m_camera->SetFOV(m_camera->GetFOV() + 1);
    elog::LogInfo(core::string::format("FOV: {}", m_camera->GetFOV()));
  }

  bool anyDirectionKeyPressed = wk | ak | sk | dk;

  if (anyDirectionKeyPressed) {
    auto sum = forwardVelocity + strafeVelocity;
    auto direction = glm::normalize(sum);
    auto totalVelocity = direction * (supaSpeed ? 10.0f : 5.0f);

    m_player->GetVelocity().x = totalVelocity.x;
    m_player->GetVelocity().z = totalVelocity.z;
  } else {
    m_player->GetVelocity().x = 0;
    m_player->GetVelocity().z = 0;
  }

  if (IsKeyDown(input::Keys::SPACE)) {
    m_player->Jump(150);
  }

  if (IsMouseButtonDown(input::MouseButtons::Left)) {
    auto [start, dir] = GetPlayerAimDirection();

    m_debugRenderer->AddLine(start, start + dir, 0.5);

    if (m_playerActor->GetAnimationController()->IsAnimationPlaying(
            "Armature|attack") == false) {
      m_playerActor->GetAnimationController()->SetAnimation(
          "Armature|attack",
          render::anim::AnimationPlaybackOptions(false, 60, 1, true));
    }
  }
}

core::tuple<glm::vec3, glm::vec3> GameState::GetPlayerAimDirection() {
  auto playerPos = m_player->GetPosition();
  glm::vec3 direction = glm::normalize(m_camera->GetDirection()) * 5.0f;
  return {playerPos, direction};
}

bool GameState::OnMouseMoveDelta(const int32_t x, const int32_t y) {
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

bool GameState::OnMouseUp(const input::MouseButton &key) {
  if (key == input::MouseButtons::Left) {
    elog::LogInfo("Mouse up");

    auto [start, dir] = GetPlayerAimDirection();
    auto end = start + dir;
    elog::LogInfo(core::string::format("start: [{}, {}, {}], end: [{}, {}, {}]",
                                       start.x, start.y, start.z, end.x, end.y,
                                       end.z));
    m_debugRenderer->AddLine(start, start + dir, 5);

    auto ci = vox::CollisionInfo(start, dir);
    m_collisionManager->Collide(ci);

    if (ci.HasCollided()) {
      uint32_t nodeX, nodeY, nodeZ;
      vox::decodeMK(ci.node.start, nodeX, nodeY, nodeZ);

      m_debugRenderer->AddAABV(glm::vec3(nodeX, nodeY, nodeZ),
                               glm::vec3(nodeX + 1, nodeY + 1, nodeZ + 1), 5);

      m_octree->RemoveNode(nodeX, nodeY, nodeZ);

      uint32_t chunk = vox::utils::GetChunk(ci.node.start);
      m_meshManager->RebuildChunk(chunk);
    }
  }

  return GameInputHandler::OnMouseUp(key);
}
bool GameState::OnMouseDown(const input::MouseButton &key) {

  return GameInputHandler::OnMouseDown(key);
}
bool GameState::OnKeyUp(const input::Key &key, const bool repeated) {
  if (key == input::Keys::GRAVE_ACCENT) {
    auto cursorMode =
        Game->GetWindow()->GetCursorMode() == render::CursorMode::Normal
            ? render::CursorMode::HiddenCapture
            : render::CursorMode::Normal;

    Game->GetWindow()->SetCursorMode(cursorMode);
  }

  return GameInputHandler::OnKeyUp(key, repeated);
}
} // namespace game::state