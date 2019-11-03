#include "geometry/SimpleGeometry.h"
#include "input/FreeCameraInputHandler.h"
#include "input/InputInc.h"
#include "render/RenderInc.h"
#include "resource_management/ResourceManagementInc.h"
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

void HandlePlayerInput(game::Player *player, input::GameInputHandler *inputHandler, float delta);

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
    wDef.Fullscreen = false;

    auto context = engine::CreateContext(wDef);
    auto window = context->GetWindow();
    auto renderer = context->GetRenderer();

    if (!window) {
        elog::LogInfo("Failed to create window");
        return -1;
    }

    res::GpuProgramManager mgr(renderer, fsPtr.get());

    auto program = mgr.LoadProgram("resources/shaders/phong_color");
    auto material = core::MakeUnique<material::BaseMaterial>(program);

    auto program_anim = mgr.LoadProgram("resources/shaders/phong_anim");
    auto material_anim = core::MakeUnique<material::BaseMaterial>(program_anim);



    window->SetCursorMode(render::CursorMode::HiddenCapture);
    renderer->SetClearColor({20, 20, 20});

    auto loader = res::IQMLoader(fsPtr);
    auto animMesh = renderer->CreateAnimatedMesh();
    loader.Load(animMesh.get(), io::Path("resources/models/ProjectSteve.iqm"));

    res::mbd::MBDLoader mbdLoader;
    auto mbdBones = mbdLoader.LoadMBD(fileSystem, io::Path("resources/models/ProjectSteve.mbd"));
    const auto &iqmData = animMesh->GetAnimationData();
    for (int i = 0; i < mbdBones.size(); i++) {
        auto &mbdBone = mbdBones[i];
        auto &iqmBone = iqmData.bones[i];
    }

    auto imgLoader = core::MakeUnique<res::ImageLoader>(fsPtr, renderer);
    auto renderContext = renderer->GetRenderContext();

    auto texture = imgLoader->LoadImage(io::Path("resources/models/steve.png"));
    material_anim->SetTexture(0, texture.get());

    auto octree = core::MakeShared<MortonOctTree>();
    auto collisionManager = core::MakeUnique<CollisionManager>(octree);
    auto voxMeshManager = core::MakeUnique<VoxMeshManager>(renderer, octree);

    octree->AddNode(MNode(0,0,0));
    octree->AddNode(MNode(0,1,0));
    octree->SortLeafNodes();

    voxMeshManager->GenAllChunks();



    util::Timer timer;
    auto camera = core::MakeShared<render::PerspectiveCamera>(16.0f / 9.0f, 45.0f);
    camera->SetRotation({0, glm::radians(-89.0f), 0});
    renderContext->SetCurrentCamera(camera);

    auto player = core::MakeUnique<game::Player>(camera, collisionManager.get(), glm::vec3(0, 0, 0), 1, 1.6);
    auto gameInputHandler = core::MakeShared<input::GameInputHandler>();

    core::pod::Vec2<int32_t> m_mouseOld = {0, 0};
    core::pod::Vec2<int32_t> m_mouseNew = {0, 0};

    gameInputHandler->SetMouseMoveHandler([&](int32_t x, int32_t y) {
        const float MouseSpeed = 0.01;
        m_mouseOld = m_mouseNew;
        m_mouseNew = {x, y};

        auto rot = camera->GetRotation();

        rot.x -= ((float) (m_mouseNew.x - m_mouseOld.x)) * MouseSpeed;
        rot.y -= ((float) (m_mouseNew.y - m_mouseOld.y)) * MouseSpeed;

        rot.y = glm::clamp(rot.y, glm::radians(-89.0f), glm::radians(89.0f));

        camera->SetRotation(rot);
    });

    context.get()->GetWindow()->GetInputDevice().lock()->AddInputHandler(
            gameInputHandler);

    static const int PhysicsUpdateRateInMilliseconds = 16;

    player->SetFlyEnabled(true);
    timer.Start();


    auto debugShader = mgr.LoadProgram("resources/shaders/debug");
    auto debugMaterial = core::MakeShared<material::BaseMaterial>(debugShader);
    debugMaterial->UseDepthTest = false;
    debugMaterial->RenderMode = material::MeshRenderMode::Lines;
    auto debugMesh = core::MakeUnique<render::debug::DebugLineMesh>(renderer->CreateBaseMesh(), debugMaterial);


    auto currentFrame = 0.0f;
    auto animatedMeshPos = glm::vec3(2, 0, 0);

    while (window->ShouldClose() == false) {

        auto delta_ms = timer.MilisecondsElapsed();
        float delta_seconds = ((float) delta_ms) / 1000.f;

        HandlePlayerInput(player.get(), gameInputHandler.get(), delta_seconds);

        if (delta_ms >= PhysicsUpdateRateInMilliseconds) {
            timer.Start();
            player->Update(delta_seconds);
            //elog::LogInfo(core::string::format("Delta %.4f s, Player location [ %.2f | %.2f | %.2f ], ground = %i",
            //    delta_seconds,  player->GetPosition().x, player->GetPosition().y, player->GetPosition().z, (int)player->OnGround()));
        }

        renderer->BeginFrame();
        currentFrame += 25.0f * delta_seconds;

        ///Render voxels
        for(const auto & mesh: voxMeshManager->GetMeshes())
        {
            renderer->RenderMesh(mesh.second.get(), material.get(), glm::mat4(1));
        }

        ///Render animated mesh
        animMesh->GetAnimationData().set_interp_frame(currentFrame);

        glm::mat4 m(1);
        m = glm::translate(m, animatedMeshPos) *
            glm::rotate(m, glm::radians(-90.0f), {1.f, 0.f, 0.0f}) *
            glm::scale(m, {1.0f, 1.0f, 1.0f});


        renderer->RenderMesh(animMesh.get(), material_anim.get(), m);

        ///Render debug mesh
        if(mbdBones.empty()) {
            debugMesh->Clear();
            const auto &animData = animMesh->GetAnimationData();
            for (int i = 0; i < mbdBones.size(); i++) {
                auto &mbdBone = mbdBones[i];

                auto start = glm::vec4(mbdBone.head, 1) * animData.current_frame[i];
                auto end = glm::vec4(mbdBone.tail, 1) * animData.current_frame[i];
                auto color = animData.bone_colors[i];

                //elog::LogInfo(core::string::format("iqm: %s, mbd: %s", iqmBone.name.c_str(), mbdBone.name.c_str()));

                debugMesh->AddLine(start, end, color);
            }

            debugMesh->Upload();
        }

        glm::mat4 m2(1);
        m2 = glm::translate(m2, animatedMeshPos) *
            glm::rotate(m2, glm::radians(-90.0f), {1.f, 0.f, 0.0f});
        renderer->RenderMesh(debugMesh->GetMesh(), debugMesh->GetMaterial(), m2);

        ///End frame
        window->SwapBuffers();
        window->PollEvents();
        renderer->EndFrame();

        if (gameInputHandler->IsKeyDown(input::Keys::Esc)) {
            window->Close();
        }
    }

    return 0;
}

static float speed = 25.0;

void HandlePlayerInput(game::Player *player, input::GameInputHandler *inputHandler, float delta) {
    auto look = player->GetCamera()->GetLocalZ();
    auto right = player->GetCamera()->GetLocalX();

    look = glm::normalize(look);
    right = glm::normalize(right);

    auto wk = inputHandler->IsKeyDown(input::Keys::W);
    auto sk = inputHandler->IsKeyDown(input::Keys::S);
    auto dk = inputHandler->IsKeyDown(input::Keys::D);
    auto ak = inputHandler->IsKeyDown(input::Keys::A);
    auto supaSpeed = inputHandler->IsKeyDown(input::Keys::X);

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

    if (inputHandler->IsKeyDown(input::Keys::F)) {
        player->SetFlyEnabled(!player->GetFlyEnabled());
    }

    if (inputHandler->IsKeyDown(input::Keys::Space)) {
        player->Jump(30.0f);
    }
    bool anyDirectionKeyPressed = wk | ak | sk | dk;

    if (anyDirectionKeyPressed) {
        auto sum = forwardVelocity + strafeVelocity;
        auto direction = glm::normalize(sum);
        auto totalVelocity = direction * (supaSpeed ? speed * 10 : speed);

        player->GetVelocity().x = totalVelocity.x;
        player->GetVelocity().z = totalVelocity.z;
        if (player->GetFlyEnabled()) player->GetVelocity().y = totalVelocity.y;
    }

    if (!anyDirectionKeyPressed && (player->OnGround() || player->GetFlyEnabled())) {
        player->GetVelocity().x = 0;
        player->GetVelocity().z = 0;
        if (player->GetFlyEnabled()) player->GetVelocity().y = 0;
    }
}