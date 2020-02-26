#ifndef THEPROJECT2_GAME_H
#define THEPROJECT2_GAME_H

#include "../../src/game/GameStateManager.h"
#include "filesystem/FileSystemInc.h"
#include "render/RenderInc.h"
#include "scene/rendering/Renderer.h"
#include "window/WindowInc.h"
#include <resource_management/ResourceManagementInc.h>

namespace game {
class CGame {
public:
  static bool Initialize(const render::SWindowDefinition &def);

public:
  CGame();
  ~CGame();

  render::IRenderer *GetRenderer() { return m_renderer; }
  scene::Renderer * GetSceneRenderer() { return m_sceneRenderer.get(); }

  render::IWindow *GetWindow() { return m_window; }

  io::IFileSystem *GetFileSystem() { return m_fileSystem.get(); }

  res::GpuProgramManager *GetGpuProgramManager() {
    return m_gpuProgramManager.get();
  }

  res::ImageLoader *GetImageLoader() { return m_imageLoader.get(); }

  game::GameStateManager *GetGameStateManager() {
    return m_gameStateManager.get();
  }

  res::ResourceManager *GetResourceManager() {
    return m_resourceManager.get();
  }

private:
  render::IRenderer *m_renderer;
  core::UniquePtr<scene::Renderer> m_sceneRenderer;
  render::IWindow *m_window;
  core::UniquePtr<res::ResourceManager> m_resourceManager;
  core::UniquePtr<io::IFileSystem> m_fileSystem;
  core::UniquePtr<engine::IEngineContext> m_engineContext;
  core::UniquePtr<res::GpuProgramManager> m_gpuProgramManager;
  core::UniquePtr<res::ImageLoader> m_imageLoader;
  core::UniquePtr<game::GameStateManager> m_gameStateManager;
  core::SharedPtr<elog::DefaultCoutLogPipe> m_coutLogPipe;
  core::UniquePtr<res::mesh::AssimpImport> m_assimpImporter;
};
} // namespace game

extern game::CGame *Game;

#endif // THEPROJECT2_GAME_H
