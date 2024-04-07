cmake_minimum_required(VERSION 3.10)
project(TheProject2)
set(BINARY ${CMAKE_PROJECT_NAME}_run)
set(BINARY_DBG ${CMAKE_PROJECT_NAME}_dbg_run)
set(LIBRARY ${CMAKE_PROJECT_NAME}_lib)

set(CPP_GCC_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fdelete-null-pointer-checks -Wall -Wno-reorder -Wno-misleading-indentation  -Werror=return-type -std=c++17 -msse4.1 -include EngineInc.h")
set(CPP_GCC_COMPILE_DBG_FLAGS "${CMAKE_CXX_FLAGS} -O1 -g -fno-omit-frame-pointer -glldb -fdelete-null-pointer-checks -Wall -Wno-reorder -Wno-misleading-indentation -Werror=return-type -std=c++17 -msse4.1 -include EngineInc.h")

#set(CPP_GCC_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -g -fdelete-null-pointer-checks -Wno-reorder -Wno-misleading-indentation -Wall -Werror=return-type -std=c++17 -include EngineInc.h")

#set(CPP_NMAKE_COMPILE_FLAGS  "${CMAKE_CXX_FLAGS} /Gm- /std:c++latest /MP /O2 /W3 /FIEngineInc.h")

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
        "${SRC_PATH}/voxel/VoxelSide.cpp"
        "${SRC_PATH}/voxel/VoxNode.cpp"
        "${SRC_PATH}/voxel/MNodeUtil.cpp"
        "${SRC_PATH}/voxel/MortonOctree.cpp"
        "${SRC_PATH}/voxel/WorldRenderer.cpp"
        "${SRC_PATH}/voxel/CollisionManager.cpp"
        "${SRC_PATH}/core/AxisAlignedBoundingBox.cpp"

        "${SRC_PATH}/Input/GameInputHandler.cpp"

        "${SRC_PATH}/game/Player.cpp"
        "${SRC_PATH}/game/Game.cpp"
        "${SRC_PATH}/game/state/AnimationPreviewState.cpp"
        "${SRC_PATH}/game/GameStateManager.cpp"
        "${SRC_PATH}/object/Actor.cpp"
        "${SRC_PATH}/object/AnimatedMeshActor.cpp"
        "${SRC_PATH}/scene/rendering/Renderer.cpp"
        "${SRC_PATH}/game/state/GameState.cpp"
        src/voxel/world/WorldGenerator.cpp
        src/voxel/VoxelMesh.cpp
        src/voxel/world/World.cpp
        src/utils/thread/Sleep.cpp src/voxel/ChunkMesher.cpp include/util/MultiDimArrayIndex.h src/game/state/voxtest/VoxTestState.cpp)


include_directories(${INCLUDE_PATH})
include_directories("libs/TheEngine2/include")
include_directories("libs/TheEngine2/third_party/glm")
include_directories("libs/TheEngine2/third_party/imgui/imgui")
include_directories("libs/FastNoise2")

add_subdirectory(libs/TheEngine2)
add_subdirectory(libs/FastNoise2)


add_executable(${BINARY} "${SRC_PATH}/main.cpp" ${PROJECT_SOURCES})
set_target_properties(${BINARY} PROPERTIES COMPILE_FLAGS "${CPP_GCC_COMPILE_FLAGS}")
target_link_libraries(${BINARY} engine FastNoise2)
add_dependencies(${BINARY} engine FastNoise2)

add_executable(${BINARY_DBG} "${SRC_PATH}/main.cpp" ${PROJECT_SOURCES})
set_target_properties(${BINARY_DBG} PROPERTIES COMPILE_FLAGS "${CPP_GCC_COMPILE_DBG_FLAGS}")
target_link_libraries(${BINARY_DBG} engine FastNoise2)
add_dependencies(${BINARY_DBG} engine FastNoise2)

# ASan
#target_compile_options(${BINARY_DBG} PRIVATE -fsanitize=address)
#target_link_options(${BINARY_DBG} PRIVATE -fsanitize=address)
# UBSan
target_compile_options(${BINARY_DBG} PRIVATE -fsanitize=undefined -fsanitize=integer -fsanitize=nullability)
target_link_options(${BINARY_DBG} PRIVATE -fsanitize=undefined -fsanitize=integer -fsanitize=nullability)

if (UNIX)
    # MSan
    target_compile_options(${BINARY_DBG} PRIVATE -fsanitize=memory -fno-optimize-sibling-calls -fPIE -pie -fno-omit-frame-pointer)
    target_link_options(${BINARY_DBG} PRIVATE -fsanitize=memory -fno-optimize-sibling-calls -fPIE -pie -fno-omit-frame-pointer)
endif (UNIX)


add_library(${LIBRARY} STATIC ${PROJECT_SOURCES})
set_target_properties(${LIBRARY} PROPERTIES COMPILE_FLAGS "${CPP_GCC_COMPILE_FLAGS}")
target_link_libraries(${LIBRARY} engine FastNoise2)
add_dependencies(${LIBRARY} engine FastNoise2)