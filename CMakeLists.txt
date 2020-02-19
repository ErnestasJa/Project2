cmake_minimum_required (VERSION 3.0)
project(TheProject2)


set(CPP_GCC_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fdelete-null-pointer-checks -Wno-reorder -Wno-misleading-indentation -Wall -Werror=return-type -std=c++17 -include EngineInc.h")
#set(CPP_GCC_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -g -fdelete-null-pointer-checks -Wno-reorder -Wno-misleading-indentation -Wall -Werror=return-type -std=c++17 -include EngineInc.h")

set(CPP_NMAKE_COMPILE_FLAGS  "${CMAKE_CXX_FLAGS} /Gm- /std:c++latest /MP /O2 /W3 /FIEngineInc.h")

#Only supporting gcc on linux, mingw(gcc)/nmake on windows
#if(WIN32 AND NOT MINGW)
#    set(CMAKE_CXX_FLAGS "${CPP_NMAKE_COMPILE_FLAGS}")
#    message(STATUS "Using nmake build flags")
#    add_definitions(-DMSVC_COMPILE)
#else()
#    set(CMAKE_CXX_FLAGS "${CPP_GCC_COMPILE_FLAGS}")
#    message(STATUS "Using gcc build flags")
#    message(STATUS "Build flags: ${CPP_GCC_COMPILE_FLAGS}")
#endif()

set(SRC_PATH "src")
set(INCLUDE_PATH "include")

file(GLOB_RECURSE PROJECT_INCLUDES
        "${SRC_PATH}/*.h"
        "${SRC_PATH}/*.hpp"
        "${SRC_PATH}/*.h"
        "${SRC_PATH}/*.hpp"
        )

set(PROJECT_SOURCES
        "${SRC_PATH}/main.cpp"
        "${SRC_PATH}/utils/Timer.cpp"
        "${SRC_PATH}/voxel/VoxelSide.cpp"
        "${SRC_PATH}/voxel/MNode.cpp"
        "${SRC_PATH}/voxel/MNodeUtil.cpp"
        "${SRC_PATH}/voxel/MortonOctree.cpp"
        "${SRC_PATH}/voxel/VoxMeshManager.cpp"
        "${SRC_PATH}/voxel/CollisionManager.cpp"
        "${SRC_PATH}/core/AxisAlignedBoundingBox.cpp"

        "${SRC_PATH}/input/GameInputHandler.cpp"

        "${SRC_PATH}/object/Player.cpp"
        "${SRC_PATH}/game/Game.cpp"
        "${SRC_PATH}/game/state/AnimationPreviewState.cpp"
        "${SRC_PATH}/game/GameStateManager.cpp"
        "${SRC_PATH}/object/Actor.cpp"
        "${SRC_PATH}/object/AnimatedMeshActor.cpp"
        "${SRC_PATH}/scene/rendering/Renderer.cpp")


include_directories (${INCLUDE_PATH})
include_directories ("libs/TheEngine2/include")
include_directories ("libs/TheEngine2/third_party/glm")

add_subdirectory (libs/TheEngine2)



add_executable(TheProject2 ${PROJECT_SOURCES})
set_target_properties(TheProject2 PROPERTIES COMPILE_FLAGS "${CPP_GCC_COMPILE_FLAGS}")
target_link_libraries(TheProject2 engine)

add_dependencies(TheProject2 engine)