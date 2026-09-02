cmake_minimum_required(VERSION 3.16)

project(c_game VERSION 0.1.0 LANGUAGES C)

include(FetchContent)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(C_GAME_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(C_GAME_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

set(FLECS_STATIC ON CACHE BOOL "Build the Flecs static library" FORCE)
set(FLECS_SHARED OFF CACHE BOOL "Do not build the Flecs shared library" FORCE)
set(FLECS_TESTS OFF CACHE BOOL "Do not build Flecs' own test suite" FORCE)
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.1.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(flecs)

function(c_game_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(C_GAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
        if(C_GAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

function(c_game_enable_sanitizers target)
    if(C_GAME_ENABLE_SANITIZERS AND NOT MSVC)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

add_library(simulation src/simulation.c)
target_include_directories(simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(simulation PRIVATE flecs::flecs_static)
c_game_set_warnings(simulation)
c_game_enable_sanitizers(simulation)

add_executable(c_game src/main.c)
target_link_libraries(c_game PRIVATE simulation)
c_game_set_warnings(c_game)
c_game_enable_sanitizers(c_game)

include(CTest)
if(BUILD_TESTING)
    add_executable(simulation_tests tests/test_simulation.c)
    target_link_libraries(simulation_tests PRIVATE simulation)
    c_game_set_warnings(simulation_tests)
    c_game_enable_sanitizers(simulation_tests)
    add_test(NAME simulation_tests COMMAND simulation_tests)
endif()
