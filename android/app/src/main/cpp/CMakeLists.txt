cmake_minimum_required(VERSION 3.22.1)
project(toggleball_android LANGUAGES CXX C)

# --- COMPILER SETTINGS ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- PATH RESOLUTION & DEPENDENCIES ---
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../.." ABSOLUTE)
message(STATUS "Engine Repository Root resolved to: ${REPO_ROOT}")

find_package(game-activity REQUIRED CONFIG)

# --- SUBDIRECTORIES ---
add_subdirectory(${REPO_ROOT}/core ${CMAKE_BINARY_DIR}/core_build)

# --- SOURCES & TARGET DEFINITION ---
# The target name 'toggleball' must match the library loaded in Kotlin/Java
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "source/*.cpp")
add_library(toggleball SHARED ${SOURCES})
target_include_directories(toggleball PRIVATE include)

# --- INCLUDES & LINKING ---
target_link_libraries(toggleball PRIVATE
        core
        game-activity::game-activity
        EGL
        GLESv3
        jnigraphics
        android
        log
)