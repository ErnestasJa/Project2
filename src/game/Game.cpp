#include "game/Game.h"
#include <platform/IPlatformFileSystem.h>

game::CGame * Game = nullptr;

namespace game {
bool CGame::Initialize(const render::SWindowDefinition &def) {


  auto appPath = platform::GetPlatformFileSystem()->GetExecutableDirectory();
  appPath = appPath.GetParentDirectory(); // for dev builds

#ifdef MSVC_COMPILE
  appPath = appPath.GetParentDirectory();
#endif

  Game = new CGame();
  Game->m_fileSystem = io::CreateFileSystem(appPath);
  Game->m_fileSystem->AddSearchDirectory(appPath);
  Game->m_fileSystem->SetWriteDirectory(appPath);

  Game->m_coutLogPipe = core::MakeShared<elog::DefaultCoutLogPipe>();
  elog::AddLogStream(Game->m_coutLogPipe);

  Game->m_engineContext = engine::CreateContext(def);
  Game->m_renderer = Game->m_engineContext->GetRenderer();
  Game->m_window = Game->m_engineContext->GetWindow();
  Game->m_gpuProgramManager = core::MakeUnique<res::GpuProgramManager>(
      Game->m_renderer, Game->m_fileSystem.get());
  Game->m_imageLoader = core::MakeUnique<res::ImageLoader>(
      Game->m_fileSystem.get(), Game->m_renderer);
  Game->m_gameStateManager = core::MakeUnique<game::GameStateManager>();
  Game->m_assimpImporter = core::MakeUnique<res::mesh::AssimpImport>(Game->GetFileSystem(), Game->GetRenderer());
  Game->m_resourceManager = core::MakeUnique<res::ResourceManager>(Game->m_imageLoader.get(), Game->GetGpuProgramManager(), Game->m_assimpImporter.get());
  Game->m_sceneRenderer = core::MakeUnique<scene::Renderer>(Game->m_renderer);

  return true;
}

CGame::CGame() {}
CGame::~CGame() {
  m_window->Close();
}
}