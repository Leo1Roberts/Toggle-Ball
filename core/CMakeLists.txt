cmake_minimum_required(VERSION 3.22.1)
project(core_lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

if(ANDROID)
    # --- ANDROID ---
    find_package(game-activity REQUIRED CONFIG)
    add_library(core STATIC ${CORE_SOURCES})

    target_include_directories(core PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            ${game-activity_INCLUDE_DIRS}
    )
    target_link_libraries(core PUBLIC game-activity::game-activity)

    # Inject the Android macro explicitly
    target_compile_definitions(core PUBLIC PLATFORM_ANDROID)

else()
    # --- DESKTOP (Linux or Windows) ---
    set(GLAD_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source/glad.c")
    add_library(core STATIC ${CORE_SOURCES} ${GLAD_SOURCE})

    target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

    # Provide a unified macro for shared desktop logic (like GLFW window handling)
    target_compile_definitions(core PUBLIC PLATFORM_DESKTOP)

    # Now split the desktop specific flavors
    if(WIN32)
        target_compile_definitions(core PUBLIC PLATFORM_WINDOWS)
    else()
        target_compile_definitions(core PUBLIC PLATFORM_LINUX)
    endif()
endif()