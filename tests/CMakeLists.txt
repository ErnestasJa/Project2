cmake_minimum_required(VERSION 3.10)
project(ProjectTests)
set(BINARY ${CMAKE_PROJECT_NAME}_tst)
set(CMAKE_CXX_STANDARD 17)


enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib GTest::gtest GTest::gtest_main)
target_link_libraries(${BINARY} PUBLIC TheProjectMain_lib)

set_target_properties(${BINARY} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -include EngineInc.h")