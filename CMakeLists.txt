cmake_minimum_required(VERSION 3.21)
project(GNVEngine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_EXTERNALPROJECT_DOWNLOAD_PARALLEL 8)
set(FETCHCONTENT_DOWNLOAD_PARALLEL 8)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Enable ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
endif()

# Use mold linker if available (fastest), otherwise lld, otherwise gold
if(UNIX AND NOT APPLE)
    find_program(MOLD_LINKER mold)
    find_program(LLD_LINKER lld)
    find_program(GOLD_LINKER gold)

    if(MOLD_LINKER)
        add_link_options(-fuse-ld=mold)
        message(STATUS "Using mold linker: ${MOLD_LINKER}")
    elseif(LLD_LINKER)
        add_link_options(-fuse-ld=lld)
        message(STATUS "Using lld linker: ${LLD_LINKER}")
    elseif(GOLD_LINKER)
        add_link_options(-fuse-ld=gold)
        message(STATUS "Using gold linker: ${GOLD_LINKER}")
    endif()
endif()

set(GLFW_VERSION "3.4")
set(VOLK_VERSION "1.4.304")
set(VMA_VERSION "v3.3.0")
set(GLM_VERSION "1.0.1")
set(FASTGLTF_VERSION "v0.8.0")
set(THREADPOOL_VERSION "v5.0.0")
set(STB_VERSION "802cd45")
set(JOLTPHYSICS_VERSION "v5.3.0")
set(ENTT_VERSION "v3.15.0")
set(IMGUI_VERSION "v1.91.9b-docking")
set(SPDLOG_VERSION "v1.15.3")
set(CEREAL_VERSION "v1.3.2")

include(FetchContent)

set(FETCH_SHALLOW TRUE)
set(FETCH_PROGRESS TRUE)

macro(Fetch name)
    FetchContent_Declare(${name}
        ${ARGN}
        GIT_SHALLOW ${FETCH_SHALLOW}
        GIT_PROGRESS ${FETCH_PROGRESS}
    )
    if(NOT ${name}_POPULATED)
        FetchContent_MakeAvailable(${name})
    endif()
endmacro()

# VULKAN
find_package(Vulkan 1.4.304 REQUIRED)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(Vulkan REQUIRED COMPONENTS glslangValidator)
endif()

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
Fetch(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG ${GLFW_VERSION}
)

# GLM
Fetch(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG ${GLM_VERSION}
)

# ImGui
Fetch(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG ${IMGUI_VERSION}
)

# EnTT
Fetch(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG ${ENTT_VERSION}
)

# JoltPhysics
set(TARGET_UNIT_TESTS OFF)
set(TARGET_HELLO_WORLD OFF)
set(TARGET_PERFORMANCE_TEST OFF)
set(TARGET_SAMPLES OFF)
Fetch(
    jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics
    GIT_TAG ${JOLTPHYSICS_VERSION}
    SOURCE_SUBDIR "Build"
)

# fastgltf
Fetch(
    fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf
    GIT_TAG ${FASTGLTF_VERSION}
)

# volk
Fetch(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk
    GIT_TAG ${VOLK_VERSION}
)

# VMA
Fetch(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG ${VMA_VERSION}
)

# thread-pool
Fetch(
    threadpool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool
    GIT_TAG ${THREADPOOL_VERSION}
)

# stb
Fetch(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
    # GIT_TAG ${STB_VERSION}
)

# spdlog
Fetch(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG ${SPDLOG_VERSION}
)
# cereal
Fetch(
    cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal
    GIT_TAG ${CEREAL_VERSION}
    SOURCE_SUBDIR ""
)

file(GLOB_RECURSE SOURCES "src/engine/*.cpp")
add_library(GNVEngine STATIC ${SOURCES})

set_target_properties(GNVEngine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "GNVEngine"
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_sources(GNVEngine PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(GNVEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${volk_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${entt_SOURCE_DIR}/single_include
    ${jolt_SOURCE_DIR}/..
    ${fastgltf_SOURCE_DIR}/include
    ${threadpool_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${cereal_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}
)

target_link_libraries(GNVEngine
    PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        fastgltf
        Jolt
        GPUOpen::VulkanMemoryAllocator
        volk::volk
        spdlog::spdlog
)

file(GLOB_RECURSE APP_SOURCES "src/app/*.cpp")
add_executable(main ${APP_SOURCES})
target_link_libraries(main PRIVATE GNVEngine)
