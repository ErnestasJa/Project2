#include "geometry/SimpleGeometry.h"
#include "input/InputInc.h"
#include "render/BaseMaterial.h"
#include "render/RenderInc.h"
#include "resource_management/GpuProgramManager.h"
#include "resource_management/ImageLoader.h"
#include "window/WindowInc.h"
#include <filesystem/IFileSystem.h>
#include <iostream>
#include <platform/IPlatformFileSystem.h>
#include "input/FreeCameraInputHandler.h"
#include "voxel/VoxMeshManager.h"
#include "utils/Noise.h"

int main() {
  std::cout << "Initializing..." << std::endl;
  auto appPath = platform::GetPlatformFileSystem()->GetExecutableDirectory();
  appPath = appPath.GetParentDirectory(); // for dev builds

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


  auto camera = core::MakeShared<render::PerspectiveCamera>(16.0 / 9.0, 45.0f);

  window->GetInputDevice().lock()->AddInputHandler(
      input::CamInputHandler::Create(camera));
  window->SetCursorMode(render::CursorMode::HiddenCapture);

  renderer->SetClearColor({155, 0, 155});

  auto imgLoader = core::MakeUnique<res::ImageLoader>(fsPtr, renderer);
  auto renderContext = renderer->GetRenderContext();
  renderContext->SetCurrentCamera(camera);

  auto texture = imgLoader->LoadImage(io::Path("resources/textures/grass.png"));
  material->SetTexture(0, texture.get());

  camera->SetPosition({40,40,40});
  camera->SetRotation({0,glm::radians(-89.0f),0});

  auto octree= core::MakeShared<MortonOctTree>();
  auto vmgr = core::MakeUnique<VoxMeshManager>(renderer, octree, 5);

  siv::PerlinNoise schnozer(12345);
  siv::PerlinNoise rgb_schnozer(2546);
  int nodeCount = 0;

  double freq = 64.0;
  double rgb_freq = 32.0;
  double octaves = 1.0;
  double rgb_octaves = 4.0;

  for(int x = 0; x < 512; x++) {
    for (int z = 0; z < 512; z++) {
      int val = 64.0 * schnozer.octaveNoise0_1(((double)x) / freq, ((double)z)/freq, octaves);

      uint8_t r = rgb_schnozer.octaveNoise0_1(x/ rgb_freq,z/rgb_freq, rgb_octaves) *255.0;
      uint8_t g = rgb_schnozer.octaveNoise0_1(z/ rgb_freq,x/rgb_freq, rgb_octaves) *255.0;

      for (int y = 0; y < val; y++) {
        uint8_t b = rgb_schnozer.octaveNoise0_1(y/ rgb_freq, rgb_octaves) *255.0;

        octree->AddOrphanNode(MNode(encodeMK(x,y,z),1, r,g,b));
        nodeCount++;
      }
    }
  }

  elog::LogInfo(core::string::CFormat("Added %i nodes.", nodeCount));

  octree->SortLeafNodes();
  vmgr->GenAllChunks();


  auto meshes = vmgr->GetMeshes();
  for(auto mesh: meshes) {
    uint32_t x, y, z;
    decodeMK(mesh.first, x, y, z);
    elog::LogInfo(core::string::CFormat("Chunk position [%i][%i][%i]", x,y,z));
  }

  while (window->ShouldClose() == false) {

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
  }

  return 0;
}