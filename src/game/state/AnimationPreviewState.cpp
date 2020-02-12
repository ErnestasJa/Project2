#include "AnimationPreviewState.h"
#include "../Game.h"
#include "render/AnimatedMesh.h"
#include "render/BaseMaterial.h"
#include "resource_management/mesh/AssimpImport.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace game::state {
const char * AnimationPreviewState::Name = "AnimationPreview";

bool AnimationPreviewState::Initialize() {
  m_assimpImporter = core::MakeUnique<res::mesh::AssimpImport>(Game->GetFileSystem(), Game->GetRenderer());

  Game->GetWindow()->SetCursorMode(render::CursorMode::HiddenCapture);
  Game->GetRenderer()->SetClearColor(render::Vec3i{155,155,255});

  LoadMaterials();

  m_camera = core::MakeShared<render::PerspectiveCamera>(16.0f / 9.0f, 45.0f);
  Game->GetRenderer()->GetRenderContext()->SetCurrentCamera(m_camera);

  m_inputHandlerHandle =
      Game->GetWindow()->GetInputDevice()->AddInputHandler(this);

  m_debugMesh = core::MakeUnique<render::debug::DebugLineMesh>(
      Game->GetRenderer()->CreateBaseMesh(), m_debugMaterial);

  auto gridMat = m_debugMaterial->Instance();
  gridMat->UseDepthTest = true;
  m_grid = core::MakeUnique<render::debug::DebugLineMesh>(
      Game->GetRenderer()->CreateBaseMesh(), gridMat);

  m_grid->AddGrid(50, 1, glm::tvec3<uint8_t>(0,0,0));
  m_grid->Upload();

  m_steve = m_assimpImporter->LoadMesh(io::Path("resources/models/ProjectSteve.fbx"));
  m_steve->GetAnimationData().SetAnimation("Armature|idle");

  m_timer.Start();
  return false;
}

void AnimationPreviewState::LoadAnimatedMesh(core::String name) {
  m_mesh->Clear();
  m_assimpImporter->LoadMesh(io::Path("resources/models/" + name));
}

void AnimationPreviewState::LoadMaterials() {
  auto program = Game->GetGpuProgramManager()->LoadProgram(
      "resources/shaders/phong_anim");
  m_animMeshMaterial = core::MakeUnique<material::BaseMaterial>(program);

  m_texture = Game->GetImageLoader()->LoadImage(io::Path("resources/models/steve.png"));
  m_animMeshMaterial->SetTexture(0, m_texture.get());

  auto debugShader = Game->GetGpuProgramManager()->LoadProgram(
      "resources/shaders/debug");
  m_debugMaterial = core::MakeShared<material::BaseMaterial>(debugShader);
  m_debugMaterial->UseDepthTest = false;
  m_debugMaterial->RenderMode = material::MeshRenderMode::Lines;

  auto phongShader = Game->GetGpuProgramManager()->LoadProgram(
      "resources/shaders/phong_color");
  m_phongMaterial = core::MakeShared<material::BaseMaterial>(phongShader);
}

bool AnimationPreviewState::Finalize() {
  return false;
}

core::String AnimationPreviewState::GetName() {
  return Name;
}

bool AnimationPreviewState::Run() {
  auto delta_ms = m_timer.MilisecondsElapsed();
  float delta_seconds = ((float)delta_ms) / 1000.f;
  m_timer.Start();

  HandleKeyInput(delta_seconds);

  //elog::LogInfo(core::string::format("delta ms: {}", delta_seconds));
  CurrentFrame += (32.0f * delta_seconds) / 5.0f;

  Render();

  return m_shouldExitState == false;
}

glm::tvec3<uint8_t> GetBoneColor(core::String name){
    if(name == "head"){
        return {255,0,0};
    }
    if(name == "chest"){
        return {0,255,0};
    }

    if(name == "armL"){
        return {0,255,255};
    }
    if(name == "armR"){
        return {255,255,0};
    }

    if(name == "legL"){
        return {0,0,255};
    }
    if(name == "legR"){
        return {255,0,0};
    }
    return {0,0,0};
}

void AnimationPreviewState::RenderBones(float time){
    auto & animData = m_steve->GetAnimationData();
    auto & bones = animData.bones;

    core::Stack<render::BoneTransform> boneIndexStack;

    for(int i = 0; i < bones.size(); i++ ){
        if(bones[i].parent < 0){
            boneIndexStack.push(render::BoneTransform {
                    i,
                    glm::mat4(1)
            });
        }
    }

    m_debugMesh->Clear();

    while(!boneIndexStack.empty()){
        auto boneInfo = boneIndexStack.top();
        boneIndexStack.pop();

        auto animTransform = animData.current_animation->BoneKeys[boneInfo.index].GetTransform(time);
        auto & bone = bones[boneInfo.index];
        auto boneTransform = animData.GlobalInverseTransform * boneInfo.ParentTransform * animTransform;
        auto boneEndTransform = animData.GlobalInverseTransform * boneTransform * bone.bone_end;


        glm::vec4 start =  boneTransform * glm::vec4(0,0,0,1);
        glm::vec4 end = boneEndTransform * glm::vec4(0,0,0,1);

        m_debugMesh->AddLine(glm::vec3(start.x, start.y, start.z), glm::vec3(end.x, end.y, end.z), GetBoneColor(bone.name));

        for(int i = 0; i < bones.size(); i++){
            if(bones[i].parent == boneInfo.index){
                boneIndexStack.push(render::BoneTransform {
                        i,
                        boneTransform
                });
            }
        }
    }

    m_debugMesh->Upload();
    Game->GetRenderer()->RenderMesh(m_debugMesh->GetMesh(), m_debugMesh->GetMaterial(), glm::mat4(1));
    //
}

void AnimationPreviewState::Render(){
  Game->GetRenderer()->BeginFrame();
  Game->GetRenderer()->Clear();

//  m_mesh->GetAnimationData().set_interp_frame(CurrentFrame);
//
//  glm::mat4 m(1);
//  m = glm::translate(m, glm::vec3(0, 0, 0)) *
//      glm::rotate(m, glm::radians(-90.0f), {1.f, 0.f, 0.0f}) *
//      glm::scale(m, {1.0f, 1.0f, 1.0f});
//
//  Game->GetRenderer()->RenderMesh(m_mesh.get(), m_animMeshMaterial.get(), m);
//
//  const auto &animData = m_mesh->GetAnimationData();
//  if(m_bones.empty() == false && m_bones.size() <= animData.bones.size()) {
//    m_debugMesh->Clear();
//    for (int i = 0; i < m_bones.size(); i++) {
//      auto &mbdBone = m_bones[i];
//
//      auto start = glm::vec4(mbdBone.head, 1) * animData.current_frame[i];
//      auto end = glm::vec4(mbdBone.tail, 1) * animData.current_frame[i];
//      auto color = animData.bone_colors[i];
//
//      m_debugMesh->AddLine(start, end, color);
//    }
//
//    m_debugMesh->Upload();
//
//    glm::mat4 m2(1);
//    m2 = glm::translate(m2, glm::vec3(0, 0, 0))
//       * glm::rotate(m2, glm::radians(-90.0f), {1.f, 0.f, 0.0f})
//       * glm::scale(m2, {1.0f, 1.0f, 1.0f});
//
//    Game->GetRenderer()->RenderMesh(m_debugMesh->GetMesh(), m_debugMesh->GetMaterial(), m2);
//  }

  if(m_steve) {
    glm::mat4 animatedTransform(1);
      animatedTransform = glm::translate(animatedTransform, glm::vec3(0, 0, 0))
         // * glm::rotate(m3, glm::radians(-90.0f), {1.f, 0.f, 0.0f})
         * glm::scale(animatedTransform, {1.0f, 1.0f, 1.0f});

    if(m_steve->GetAnimationData().current_animation){
        if(utils::math::gequal(CurrentFrame, m_steve->GetAnimationData().current_animation->duration)){
            CurrentFrame = 0;
        }
    }

    m_steve->GetAnimationData().Animate(CurrentFrame);
    Game->GetRenderer()->RenderMesh(m_steve.get(), m_animMeshMaterial.get(), animatedTransform);

    /*glm::mat4 regularTransform(1);
    regularTransform = glm::translate(regularTransform, glm::vec3(-5, 5, 0))
                      * glm::scale(regularTransform, {1.0f, 1.0f, 1.0f});
    Game->GetRenderer()->RenderMesh(m_steve.get(), m_phongMaterial.get(), regularTransform);*/
    Game->GetRenderer()->RenderMesh(m_grid->GetMesh(), m_grid->GetMaterial(), glm::mat4(1));
    RenderBones(CurrentFrame);
  }
}

void AnimationPreviewState::HandleKeyInput(float deltaSeconds) {
  m_shouldExitState |= IsKeyDown(input::Keys::Esc);

  if(IsKeyDown(input::Keys::R)){
      LoadAnimatedMesh("mr_fixit");
  }
  else if(IsKeyDown(input::Keys::T)){
      LoadAnimatedMesh("ProjectSteve");
  }

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

  bool anyDirectionKeyPressed = wk | ak | sk | dk;

  if (anyDirectionKeyPressed) {
    auto sum = forwardVelocity + strafeVelocity;
    auto direction = glm::normalize(sum);
    auto totalVelocity =
        direction * (supaSpeed ? CameraSpeed * 10 : CameraSpeed);

    auto currentPos = m_camera->GetPosition();
    currentPos += totalVelocity;
    m_camera->SetPosition(currentPos);
  }
}

bool AnimationPreviewState::OnMouseMoveDelta(const int32_t x,
                                             const int32_t y) {
  auto rot = m_camera->GetRotation();
  float mouseSpeed = 0.01;
  rot.x -= x * mouseSpeed;
  rot.y -= y * mouseSpeed;

  rot.y = glm::clamp(rot.y, glm::radians(-89.0f), glm::radians(89.0f));

  m_camera->SetRotation(rot);
  return true;
}

}