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

# Enable extensive warnings for our code without turning them into errors.
if(MSVC)
    target_compile_options(game PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    target_compile_options(game PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wcast-qual
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wundef
        -Wmissing-declarations
        -Wswitch-enum
        -Wzero-as-null-pointer-constant
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(game PRIVATE
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
        )
    endif()
endif()

target_link_libraries(game PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)
#target_link_libraries(game PRIVATE flecs::flecs_static)
