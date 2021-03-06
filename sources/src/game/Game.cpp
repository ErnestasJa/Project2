#include "game/Game.h"
#include <platform/IPlatformFileSystem.h>
#include "gui/IGui.h"

game::CGame * Game = nullptr;

namespace game {
bool CGame::Initialize(const render::SWindowDefinition &def) {
  auto appPath = platform::GetPlatformFileSystem()->GetExecutableDirectory();
  appPath = appPath.GetParentDirectory().GetParentDirectory(); // for dev builds

#ifdef MSVC_COMPILE
  appPath = appPath.GetParentDirectory();
#endif

  Game = new CGame();
  Game->m_fileSystem = io::CreateFileSystem(appPath);
  Game->m_fileSystem->AddSearchDirectory(appPath);
  Game->m_fileSystem->SetWriteDirectory(appPath);

  Game->m_coutLogPipe = core::MakeShared<elog::DefaultCoutLogPipe>();
  elog::AddLogStream(Game->m_coutLogPipe);

  if(!Game->m_fileSystem->DirectoryExists(io::Path("resources"))){
    const auto& searchPath = appPath.AsString();
    elog::LogInfo(core::string::format("Could not find resource directory, current search path: <{}>", searchPath.c_str()));
    return false;
  }

  Game->m_engineContext = engine::CreateContext(def);
  Game->m_renderer = Game->m_engineContext->GetRenderer();
  Game->m_window = Game->m_engineContext->GetWindow();

  Game->m_imageLoader = core::MakeUnique<res::ImageLoader>(
      Game->m_fileSystem.get(), Game->m_renderer);
  Game->m_gameStateManager = core::MakeUnique<game::GameStateManager>();
  Game->m_assimpImporter = core::MakeUnique<res::mesh::AssimpImport>(Game->GetFileSystem(), Game->GetRenderer());
  Game->m_resourceManager = core::MakeUnique<res::ResourceManager>(
          Game->m_imageLoader.get(), Game->m_renderer, Game->m_fileSystem.get(), Game->m_assimpImporter.get());
  Game->m_sceneRenderer = core::MakeUnique<scene::Renderer>(Game->m_renderer);
  Game->m_gui = gui::CreateGui(Game->m_engineContext.get());
  return true;
}

CGame::CGame() {

}

CGame::~CGame() {
    m_coutLogPipe = nullptr;
    elog::ClearStreams();
    elog::ClearStreams();
    m_sceneRenderer = nullptr;
    m_assimpImporter = nullptr;
    m_resourceManager = nullptr;
    m_window->Close();
}
}