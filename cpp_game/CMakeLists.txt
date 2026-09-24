cmake_minimum_required(VERSION 3.16)

project(game LANGUAGES CXX)

find_package(SDL3 3.2 REQUIRED CONFIG)
find_package(SDL3_image 3.2 REQUIRED CONFIG)

#include(FetchContent)

#set(FLECS_SHARED OFF CACHE BOOL "" FORCE)
#FetchContent_Declare( flecs GIT_REPOSITORY https://github.com/SanderMertens/flecs.git GIT_TAG v4.1.5 GIT_SHALLOW TRUE)
#FetchContent_MakeAvailable(flecs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(game ${GAME_SOURCES})
target_link_libraries(game PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)
#target_link_libraries(game PRIVATE flecs::flecs_static)
