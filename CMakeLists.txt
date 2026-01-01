cmake_minimum_required(VERSION 3.27)
project(gramarye-chunk-renderer C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# WebAssembly toggle
option(BUILD_WEB "Build with Emscripten (WebAssembly)" OFF)

if(EMSCRIPTEN OR BUILD_WEB)
    set(PLATFORM Web CACHE STRING "" FORCE)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Option to build as shared or static (default static)
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# Fetch gramarye-libcore
include(FetchContent)
FetchContent_Declare(
    gramarye_libcore
    GIT_REPOSITORY "https://github.com/noahspoling/gramarye-libcore.git"
    GIT_TAG "main"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

# Check if target already exists (from parent using local modules)
if(NOT TARGET gramarye-libcore)
    if(USE_LOCAL_MODULES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-libcore/CMakeLists.txt")
        message(STATUS "gramarye-chunk-renderer: Using local gramarye-libcore")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-libcore gramarye-libcore)
    else()
        FetchContent_MakeAvailable(gramarye_libcore)
    endif()
endif()

# Fetch gramarye-ecs
FetchContent_Declare(
    gramarye_ecs
    GIT_REPOSITORY "https://github.com/noahspoling/gramarye-ecs.git"
    GIT_TAG "main"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

# Check if target already exists (from parent using local modules)
if(NOT TARGET gramarye-ecs)
    if(USE_LOCAL_MODULES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-ecs/CMakeLists.txt")
        message(STATUS "gramarye-chunk-renderer: Using local gramarye-ecs")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-ecs gramarye-ecs)
    else()
        FetchContent_MakeAvailable(gramarye_ecs)
    endif()
endif()

# Fetch gramarye-components (for ChunkRenderData, Observer structs)
if(NOT TARGET gramarye-components)
    FetchContent_Declare(
        gramarye_components
        GIT_REPOSITORY "https://github.com/noahspoling/gramarye-components.git"
        GIT_TAG "main"
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
    )
    if(USE_LOCAL_MODULES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-components/CMakeLists.txt")
        message(STATUS "gramarye-chunk-renderer: Using local gramarye-components")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-components gramarye-components)
    else()
        FetchContent_MakeAvailable(gramarye_components)
    endif()
else()
    message(STATUS "gramarye-components already available (added by parent project)")
endif()

# Fetch gramarye-component-functions (for Position_get, Atlas_getRect)
if(NOT TARGET gramarye-component-functions)
    FetchContent_Declare(
        gramarye_component_functions
        GIT_REPOSITORY "https://github.com/noahspoling/gramarye-component-functions.git"
        GIT_TAG "main"
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
    )
    if(USE_LOCAL_MODULES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-component-functions/CMakeLists.txt")
        message(STATUS "gramarye-chunk-renderer: Using local gramarye-component-functions")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-component-functions gramarye-component-functions)
    else()
        FetchContent_MakeAvailable(gramarye_component_functions)
    endif()
else()
    message(STATUS "gramarye-component-functions already available (added by parent project)")
endif()

# Fetch gramarye-renderer-interface (for Renderer interface)
if(NOT TARGET gramarye-renderer-interface)
    FetchContent_Declare(
        gramarye_renderer_interface
        GIT_REPOSITORY "https://github.com/noahspoling/gramarye-renderer-interface.git"
        GIT_TAG "main"
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
    )
    if(USE_LOCAL_MODULES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-renderer-interface/CMakeLists.txt")
        message(STATUS "gramarye-chunk-renderer: Using local gramarye-renderer-interface")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gramarye-renderer-interface gramarye-renderer-interface)
    else()
        FetchContent_MakeAvailable(gramarye_renderer_interface)
    endif()
else()
    message(STATUS "gramarye-renderer-interface already available (added by parent project)")
endif()

# Collect source files
file(GLOB SRC_FILES "src/*.c")

# Create library
add_library(gramarye-chunk-renderer 
    ${SRC_FILES}
)

# Add include directories
# Note: When linking against gramarye-component-functions and gramarye-components with PUBLIC,
# their PUBLIC include directories are automatically propagated via target_link_libraries.
target_include_directories(gramarye-chunk-renderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link against dependencies
# Note: Link order matters - component-functions should come after components
# since it depends on components' struct definitions
target_link_libraries(gramarye-chunk-renderer PUBLIC 
    gramarye-libcore
    gramarye-ecs
    gramarye-components
    gramarye-component-functions
    gramarye-renderer-interface
)

# Explicitly propagate include directories from dependencies
# This ensures headers are available even if automatic propagation has issues
get_target_property(COMPONENT_FUNCTIONS_INCLUDES gramarye-component-functions INTERFACE_INCLUDE_DIRECTORIES)
if(COMPONENT_FUNCTIONS_INCLUDES)
    target_include_directories(gramarye-chunk-renderer PRIVATE ${COMPONENT_FUNCTIONS_INCLUDES})
endif()
get_target_property(COMPONENTS_INCLUDES gramarye-components INTERFACE_INCLUDE_DIRECTORIES)
if(COMPONENTS_INCLUDES)
    target_include_directories(gramarye-chunk-renderer PRIVATE ${COMPONENTS_INCLUDES})
endif()

# Raylib should be linked by parent project, but we need its headers
# The parent project should set raylib_SOURCE_DIR before including this

# Export for CMake
option(GRAMARYE_CHUNK_RENDERER_INSTALL "Install gramarye-chunk-renderer" OFF)

if(GRAMARYE_CHUNK_RENDERER_INSTALL)
    include(GNUInstallDirs)
    install(TARGETS gramarye-chunk-renderer
        EXPORT chunkRendererTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT chunkRendererTargets
        FILE chunkRendererTargets.cmake
        NAMESPACE gramarye-chunk-renderer::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/chunk-renderer
    )
endif()

# For FetchContent compatibility
if(NOT TARGET gramarye-chunk-renderer::gramarye-chunk-renderer)
    add_library(gramarye-chunk-renderer::gramarye-chunk-renderer ALIAS gramarye-chunk-renderer)
endif()

