cmake_minimum_required(VERSION 3.22.1)
project("toggleball_desktop" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compute repo root relative to this folder
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
message(STATUS "Engine Repository Root resolved to: ${REPO_ROOT}")

# Add cross-platform core library as a subproject
add_subdirectory(${REPO_ROOT}/core ${CMAKE_BINARY_DIR}/core_build)

# Local entry point source file
set(DESKTOP_SOURCES main.cpp
        ../core/include/AssetManager.h
        ../core/source/AssetManager.cpp
)
add_executable(toggleball_desktop ${DESKTOP_SOURCES})

# Link shared engine logic
target_link_libraries(toggleball_desktop PRIVATE core)

if(WIN32)
    # --- WINDOWS ENVIRONMENT SPECIFICS ---
    if(NOT DEFINED GLFW_DIR)
        # Fallback default location for your Windows GLFW binaries
        set(GLFW_DIR "C:/dev/glfw-3.4.bin.WIN64")
    endif()

    target_include_directories(toggleball_desktop PRIVATE "${GLFW_DIR}/include")
    target_link_directories(toggleball_desktop PRIVATE "${GLFW_DIR}/lib-vc2022")
    target_link_libraries(toggleball_desktop PRIVATE glfw3 opengl32)
else()
    # --- LINUX ENVIRONMENT SPECIFICS ---
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)

    target_link_libraries(toggleball_desktop PRIVATE
        glfw
        OpenGL::GL
    )
endif()
