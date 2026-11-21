cmake_minimum_required(VERSION 3.16)
project(VaultHeist LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
include(FetchContent)
FetchContent_Declare(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 3.0.0 GIT_SHALLOW ON)
FetchContent_MakeAvailable(SFML)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(GLOB_RECURSE MODEL_SOURCES CONFIGURE_DEPENDS src/model/*.cpp src/util/*.cpp)
file(GLOB_RECURSE GAME_CORE_SOURCES CONFIGURE_DEPENDS src/game/*.cpp)
file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS src/gui/*.cpp)
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS bench/*.cpp)
add_library(model STATIC ${MODEL_SOURCES})
target_include_directories(model PUBLIC src)
add_library(gamecore STATIC ${GAME_CORE_SOURCES})
target_include_directories(gamecore PUBLIC src)
target_link_libraries(gamecore PUBLIC model)
add_executable(vaultheist src/main.cpp ${GUI_SOURCES})
target_include_directories(vaultheist PRIVATE src)
target_link_libraries(vaultheist PRIVATE gamecore SFML::Graphics)
add_custom_command(TARGET vaultheist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data)
add_executable(tests ${TEST_SOURCES})
target_sources(tests PRIVATE
    src/gui/core/Renderer.cpp
    src/gui/widgets/HeapView.cpp)
target_link_libraries(tests PRIVATE gamecore SFML::Graphics)
add_executable(bench ${BENCH_SOURCES})
target_link_libraries(bench PRIVATE model)
set(RENDERDEMO_SOURCES
    src/gui/core/Camera.cpp
    src/gui/core/Renderer.cpp)
add_executable(renderdemo tools/render_demo.cpp ${RENDERDEMO_SOURCES})
target_include_directories(renderdemo PRIVATE src)
target_link_libraries(renderdemo PRIVATE model SFML::Graphics)
add_executable(heapviewdemo
    tools/heap_view_demo.cpp
    src/gui/core/Renderer.cpp
    src/gui/widgets/HeapView.cpp)
target_include_directories(heapviewdemo PRIVATE src)
target_link_libraries(heapviewdemo PRIVATE model SFML::Graphics)
add_executable(treeanimdemo
    tools/tree_anim_demo.cpp
    src/gui/core/Renderer.cpp
    src/gui/tree/TreeAnimationView.cpp)
target_include_directories(treeanimdemo PRIVATE src)
target_link_libraries(treeanimdemo PRIVATE gamecore SFML::Graphics)
