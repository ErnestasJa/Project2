cmake_minimum_required (VERSION 3.0)
project(TheProject2)


set(CPP_GCC_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -g -O1 -w -std=c++17 -include EngineInc.h")
set(CPP_NMAKE_COMPILE_FLAGS  "${CMAKE_CXX_FLAGS} /Gm- /MP /O2 /W3 /FIEngineInc.h")

#Only supporting gcc on linux, mingw(gcc)/nmake on windows
if(WIN32 AND NOT MINGW)
    set(CMAKE_CXX_FLAGS "${CPP_NMAKE_COMPILE_FLAGS}")
else()
    set(CMAKE_CXX_FLAGS "${CPP_GCC_COMPILE_FLAGS}")
endif()

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
        include/utils/Noise.h)


include_directories (${INCLUDE_PATH})
include_directories ("libs/TheEngine2/include")
include_directories ("libs/TheEngine2/third_party/glm")

add_subdirectory (libs/TheEngine2/third_party/glad)
add_subdirectory (libs/TheEngine2/third_party/glfw)
add_subdirectory (libs/TheEngine2/third_party/imgui)
add_subdirectory (libs/TheEngine2/third_party/physfs)
add_subdirectory (libs/TheEngine2)



add_executable(TheProject2 ${PROJECT_SOURCES})

target_link_libraries(TheProject2 engine glad glfw imgui physfs-static)