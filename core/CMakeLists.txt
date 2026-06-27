cmake_minimum_required(VERSION 3.22.1)
project(toggleball_core LANGUAGES CXX C)

# --- COMPILER SETTINGS ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- SOURCES & TARGET DEFINITION ---
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

if(ANDROID)
    find_package(game-activity REQUIRED CONFIG)
    add_library(core STATIC ${CORE_SOURCES})
else()
    # Desktop also requires the OpenGL loader
    set(GLAD_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source/glad.c")
    add_library(core STATIC ${CORE_SOURCES} ${GLAD_SOURCE})
endif()

# --- INCLUDES & LINKING ---
target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(ANDROID)
    target_include_directories(core PUBLIC ${game-activity_INCLUDE_DIRS})
    target_link_libraries(core PUBLIC game-activity::game-activity)

    # Inject the Android platform macro
    target_compile_definitions(core PUBLIC PLATFORM_ANDROID)
else()
    # Inject the shared Desktop platform macro
    target_compile_definitions(core PUBLIC PLATFORM_DESKTOP)

    # Split the desktop-specific flavors
    if(WIN32)
        target_compile_definitions(core PUBLIC PLATFORM_WINDOWS)
    else()
        target_compile_definitions(core PUBLIC PLATFORM_LINUX)
    endif()
endif()