cmake_minimum_required(VERSION 3.10)
project(TheProjectMain)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(sources)
include_directories(sources/include)
include_directories(sources/libs/TheEngine2/include)
include_directories(sources/libs/TheEngine2/third_party/glm)
add_subdirectory(tests)
