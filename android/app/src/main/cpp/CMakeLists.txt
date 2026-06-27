# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("toggleball")

# compute repo root relative to this CMakeLists
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../.." ABSOLUTE)
message(STATUS "Repo root: ${REPO_ROOT}")

# add core as a subproject
add_subdirectory(${REPO_ROOT}/core ${CMAKE_BINARY_DIR}/core_build)

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(toggleball SHARED
        main.cpp)

# your existing target (toggleball) remains; link core
target_link_libraries(toggleball PRIVATE core
		game-activity::game-activity

        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        jnigraphics
        android
        log)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)