#include "geometry/SimpleGeometry.h"
#include "input/FreeCameraInputHandler.h"
#include "input/InputInc.h"
#include "render/BaseMaterial.h"
#include "render/RenderInc.h"
#include "resource_management/GpuProgramManager.h"
#include "resource_management/ImageLoader.h"
#include "util/Noise.h"
#include "voxel/Morton.h"
#include "voxel/VoxMeshManager.h"
#include "window/WindowInc.h"
#include <filesystem/IFileSystem.h>
#include <game/Player.h>
#include <iostream>
#include <platform/IPlatformFileSystem.h>
#include <util/Timer.h>
#include "input/GameInputHandler.h"

void HandlePlayerInput(game::Player* player, input::GameInputHandler* inputHandler, float delta);

int main() {
  std::cout << "Initializing..." << std::endl;
  auto appPath = platform::GetPlatformFileSystem()->GetExecutableDirectory();
  appPath = appPath.GetParentDirectory(); // for dev builds

#ifdef MSVC_COMPILE
  appPath = appPath.GetParentDirectory();
#endif

  auto fsPtr = io::CreateFileSystem(appPath);
  auto fileSystem = fsPtr.get();
  fileSystem->AddSearchDirectory(appPath);
  fileSystem->SetWriteDirectory(appPath);

  auto engineLogStream = core::MakeShared<elog::DefaultCoutLogPipe>();
  elog::AddLogStream(engineLogStream);

  render::SWindowDefinition wDef;
  wDef.Dimensions = {1280, 720};
  wDef.Title = "TheProject2";

  auto context = engine::CreateContext(wDef);
  auto window = context->GetWindow();
  auto renderer = context->GetRenderer();

  res::GpuProgramManager mgr(renderer, fsPtr.get());

  auto program = mgr.LoadProgram("resources/shaders/phong_color");
  auto material = core::MakeUnique<material::BaseMaterial>(program);
  material->SetI("DiffuseTextureSampler", 0);

  if (!window) {
    std::cout << "Failed to create window" << std::endl;
    return -1;
  }



  window->SetCursorMode(render::CursorMode::HiddenCapture);

  renderer->SetClearColor({155, 0, 155});

  auto imgLoader = core::MakeUnique<res::ImageLoader>(fsPtr, renderer);
  auto renderContext = renderer->GetRenderContext();

  auto texture = imgLoader->LoadImage(io::Path("resources/textures/grass.png"));
  material->SetTexture(0, texture.get());

  auto octree= core::MakeShared<MortonOctTree>();
  auto vmgr = core::MakeUnique<VoxMeshManager>(renderer, octree, 5);

  siv::PerlinNoise schnozer(12345);
  siv::PerlinNoise rgb_schnozer(2546);
  int nodeCount = 0;


  double freq = 64.0;
  double rgb_freq = 32.0;
  double octaves = 1.0;
  double rgb_octaves = 2.0;

  const int world_size = 512;
  const int height = 64;

  for(int x = 0; x < world_size; x++) {
    for (int z = 0; z < world_size; z++) {
      int val = ((double)height) * schnozer.octaveNoise0_1(((double)x) / freq, ((double)z)/freq, octaves);

      uint8_t r = rgb_schnozer.octaveNoise0_1(x/ rgb_freq,z/rgb_freq, rgb_octaves) *255.0;
      uint8_t g = rgb_schnozer.octaveNoise0_1(z/ rgb_freq,x/rgb_freq, rgb_octaves) *255.0;

      for (int y = 0; y < val; y++) {
        uint8_t b = rgb_schnozer.octaveNoise0_1(y/ rgb_freq, rgb_octaves) *255.0;

        octree->AddOrphanNode(MNode(encodeMK(x,y,z),1, r,g,b));
        nodeCount++;
      }
    }
  }



  octree->SortLeafNodes();

  util::Timer timer;

  timer.Start();
  vmgr->GenAllChunks();
  elog::LogInfo(core::string::CFormat("Added %i nodes. Took %i ms", nodeCount, timer.MilisecondsElapsed()));

  auto meshes = vmgr->GetMeshes();
  for(auto mesh: meshes) {
    uint32_t x, y, z;
    decodeMK(mesh.first, x, y, z);
    //elog::LogInfo(core::string::CFormat("Chunk position [%i][%i][%i]", x,y,z));
  }


  auto collisionManager = new CollisionManager(octree);

  auto camera = core::MakeShared<render::PerspectiveCamera>(16.0f / 9.0f, 45.0f);
  camera->SetRotation({0,glm::radians(-89.0f),0});
  renderContext->SetCurrentCamera(camera);

  auto player = core::MakeUnique<game::Player>(camera, collisionManager, glm::vec3(256, 68, 256), 1, 1.6);
  auto gameInputHandler = core::MakeShared<input::GameInputHandler>();

  core::pod::Vec2<int32_t> m_mouseOld={0,0};
  core::pod::Vec2<int32_t> m_mouseNew={0,0};

  gameInputHandler->SetMouseMoveHandler([&](int32_t x, int32_t y){
    const float MouseSpeed = 0.01;
    m_mouseOld = m_mouseNew;
    m_mouseNew = {x, y};

    auto rot = camera->GetRotation();

    rot.x -= ((float)(m_mouseNew.x - m_mouseOld.x)) * MouseSpeed;
    rot.y -= ((float)(m_mouseNew.y - m_mouseOld.y)) * MouseSpeed;

    rot.y = glm::clamp(rot.y, glm::radians(-89.0f), glm::radians(89.0f));

    camera->SetRotation(rot);
  });

  context.get()->GetWindow()->GetInputDevice().lock()->AddInputHandler(
      gameInputHandler);

  static const int PhysicsUpdateRateInMilliseconds = 16;

  player->SetFlyEnabled(true);
  timer.Start();

  while (window->ShouldClose() == false) {

    auto delta_ms = timer.MilisecondsElapsed();
    float delta_seconds = ((float)delta_ms) / 1000.f;

    HandlePlayerInput(player.get(), gameInputHandler.get(), delta_seconds);

    if(delta_ms >= PhysicsUpdateRateInMilliseconds) {
      timer.Start();
      player->Update(delta_seconds);
      //elog::LogInfo(core::string::CFormat("Delta %.4f s, Player location [ %.2f | %.2f | %.2f ], ground = %i",
      //    delta_seconds,  player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, (int)player->OnGround()));
    }

    renderer->BeginFrame();

    auto meshes = vmgr->GetMeshes();

    for(auto mesh: meshes){
      uint32_t x, y, z;
      decodeMK(mesh.first,x,y,z);
      renderer->RenderMesh(mesh.second.get(), material.get(), glm::vec3(0,0,0));
    }

    window->SwapBuffers();
    window->PollEvents();
    renderer->EndFrame();

    if(gameInputHandler->IsKeyDown(input::Keys::Esc)){
      window->Close();
    }
  }

  return 0;
}

static float speed = 25.0;

void HandlePlayerInput(game::Player* player, input::GameInputHandler* inputHandler, float delta)
{
  auto look = player->GetCamera()->GetLocalZ();
  auto right = player->GetCamera()->GetLocalX();

  look = glm::normalize(look);
  right = glm::normalize(right);

  auto wk = inputHandler->IsKeyDown(input::Keys::W);
  auto sk = inputHandler->IsKeyDown(input::Keys::S);
  auto dk = inputHandler->IsKeyDown(input::Keys::D);
  auto ak = inputHandler->IsKeyDown(input::Keys::A);

  glm::vec3 forwardVelocity(0), strafeVelocity(0);

  if (wk)
  {
    forwardVelocity.x = look.x;
    forwardVelocity.y = look.y;
    forwardVelocity.z = look.z;
  }
  else if (sk)
  {
    forwardVelocity.x = -look.x;
    forwardVelocity.y = -look.y;
    forwardVelocity.z = -look.z;
  }

  if (dk)
  {
    strafeVelocity.x = right.x;
    strafeVelocity.y = right.y;
    strafeVelocity.z = right.z;
  }
  else if (ak)
  {
    strafeVelocity.x = -right.x;
    strafeVelocity.y = -right.y;
    strafeVelocity.z = -right.z;
  }

  if(inputHandler->IsKeyDown(input::Keys::F)){
    player->SetFlyEnabled(!player->GetFlyEnabled());
  }

  if(inputHandler->IsKeyDown(input::Keys::Space)){
    player->Jump(30.0f);
  }
  bool anyDirectionKeyPressed = wk | ak | sk | dk;

  if(anyDirectionKeyPressed){
    auto sum = forwardVelocity + strafeVelocity;
    auto direction = glm::normalize(sum);
    auto totalVelocity = direction * speed;

    player->GetVelocity().x = totalVelocity.x;
    player->GetVelocity().z = totalVelocity.z;
    if(player->GetFlyEnabled()) player->GetVelocity().y = totalVelocity.y;
  }

  if (!anyDirectionKeyPressed && (player->OnGround() || player->GetFlyEnabled()))
  {
    player->GetVelocity().x = 0;
    player->GetVelocity().z = 0;
    if(player->GetFlyEnabled()) player->GetVelocity().y = 0;
  }
}