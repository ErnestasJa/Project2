#ifndef THEPROJECT2_GAME_H
#define THEPROJECT2_GAME_H

#include <resource_management/ResourceManagementInc.h>
#include "render/RenderInc.h"
#include "filesystem/FileSystemInc.h"
#include "window/WindowInc.h"
#include "GameStateManager.h"

namespace game {
    class CGame {
    public:
        static bool Initialize(const render::SWindowDefinition& def);

    public:
        CGame();
        ~CGame();

        render::IRenderer * GetRenderer(){
            return m_renderer;
        }

        render::IWindow * GetWindow(){
            return m_window;
        }

        io::IFileSystem * GetFileSystem(){
            return m_fileSystem.get();
        }

        res::GpuProgramManager * GetGpuProgramManager(){
            return m_gpuProgramManager.get();
        }

        res::ImageLoader * GetImageLoader(){
            return m_imageLoader.get();
        }

        game::GameStateManager* GetGameStateManager(){
            return m_gameStateManager.get();
        }

    private:
        render::IRenderer* m_renderer;
        render::IWindow * m_window;
        core::UniquePtr<io::IFileSystem> m_fileSystem;
        core::UniquePtr<engine::IEngineContext> m_engineContext;
        core::UniquePtr<res::GpuProgramManager> m_gpuProgramManager;
        core::UniquePtr<res::ImageLoader> m_imageLoader;
        core::UniquePtr<game::GameStateManager> m_gameStateManager;
        core::SharedPtr<elog::DefaultCoutLogPipe> m_coutLogPipe;
    };
}

extern game::CGame* Game;

#endif //THEPROJECT2_GAME_H
