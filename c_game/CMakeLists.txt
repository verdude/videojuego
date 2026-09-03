cmake_minimum_required(VERSION 3.16)

project(c_game LANGUAGES C)

include(FetchContent)

set(FLECS_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.1.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(flecs)

# Pick up main.c and any future modules added below src/.
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "src/*.c")

add_executable(c_game ${GAME_SOURCES})
target_compile_features(c_game PRIVATE c_std_11)
target_include_directories(c_game PRIVATE include)
target_link_libraries(c_game PRIVATE flecs::flecs_static)
