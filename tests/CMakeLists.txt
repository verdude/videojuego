find_package(GTest 1.14 CONFIG QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    # GoogleTest v1.17.0, pinned to its release commit.
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Build only the recognizer and its dependencies, so work in the fighter or
# renderer cannot prevent running the input tests. SDL is needed for headers;
# the tests supply logical input and never initialize SDL or open a window.
add_executable(cmd_recognizer_tests
    cmd_recognizer_test.cpp
    ../src/combat/commandrecognizer.cpp
    ../src/combat/command.cpp
    ../src/input/held.cpp
)
target_include_directories(cmd_recognizer_tests PRIVATE ../src)
target_link_libraries(cmd_recognizer_tests PRIVATE
    GTest::gtest_main
    SDL3::SDL3
    nlohmann_json::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(cmd_recognizer_tests PROPERTIES LABELS recognizer)
