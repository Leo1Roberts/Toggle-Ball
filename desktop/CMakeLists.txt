cmake_minimum_required(VERSION 3.22.1)
project(toggleball_desktop LANGUAGES CXX C)

# --- COMPILER SETTINGS ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- PATH RESOLUTION & DEPENDENCIES ---
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(WIN32)
    if(NOT DEFINED GLFW_DIR)
        set(GLFW_DIR "${REPO_ROOT}/third-party/GLFW")
    endif()
    message(STATUS "GLFW Include Path resolved to: ${GLFW_DIR}/include")
else()
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
endif()

# --- SUBDIRECTORIES ---
add_subdirectory(${REPO_ROOT}/core ${CMAKE_BINARY_DIR}/core_build)

# --- SOURCES & TARGET DEFINITION ---
set(DESKTOP_SOURCES main.cpp)
add_executable(toggleball_desktop ${DESKTOP_SOURCES})

# --- ASSETS PATH MACRO ---
target_compile_definitions(core PUBLIC ASSETS_PATH_MACRO=\"${REPO_ROOT}/core/assets/\")

# --- INCLUDES & LINKING ---
target_link_libraries(toggleball_desktop PRIVATE core)

if(WIN32)
    # Give core access to the Windows headers for its internal static objects
    target_include_directories(core PUBLIC "${GLFW_DIR}/include")

    # Give desktop access to headers and compiled binaries
    target_include_directories(toggleball_desktop PRIVATE "${GLFW_DIR}/include")
    target_link_directories(toggleball_desktop PRIVATE "${GLFW_DIR}/lib")
    target_link_libraries(toggleball_desktop PRIVATE glfw3 opengl32)
else()
    target_link_libraries(toggleball_desktop PRIVATE
            glfw
            OpenGL::GL
    )
endif()