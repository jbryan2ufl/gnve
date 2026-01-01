set(CMAKE_EXTERNALPROJECT_DOWNLOAD_PARALLEL 8)
set(FETCHCONTENT_DOWNLOAD_PARALLEL 8)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

set(GLFW_VERSION "3.4")
set(VMA_VERSION "v3.3.0")
set(GLM_VERSION "1.0.1")
set(FASTGLTF_VERSION "v0.8.0")
set(THREADPOOL_VERSION "v5.0.0")
set(STB_VERSION "master")
set(JOLTPHYSICS_VERSION "v5.3.0")
set(ENTT_VERSION "v3.15.0")
set(IMGUI_VERSION "v1.92.5-docking")
set(SPDLOG_VERSION "v1.15.3")
set(CEREAL_VERSION "v1.3.2")
set(KTX_VERSION "v4.4.2")
set(MESHOPTIMIZER_VERSION "v1.0")
set(SPIRVREFLECT_VERSION "main")

set(FETCH_SHALLOW ON)
set(FETCH_PROGRESS OFF)

include(FetchContent)

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
set(ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
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
    EXCLUDE_FROM_ALL
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
    GIT_TAG ${STB_VERSION}
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

# KTX
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_JS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
Fetch(
    ktx
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software
    GIT_TAG ${KTX_VERSION}
    EXCLUDE_FROM_ALL
)

# basisu
Fetch(
    basisu
    GIT_REPOSITORY https://github.com/BinomialLLC/basis_universal
    SOURCE_SUBDIR ""
)

# meshoptimizer
set(MESHOPT_BUILD_GLTFPACK ON CACHE BOOL "" FORCE)
set(MESHOPT_INSTALL OFF CACHE BOOL "" FORCE)
set(MESHOPT_GLTFPACK_BASISU_PATH ${basisu_SOURCE_DIR})
set(EXAMPLES OFF CACHE BOOL "" FORCE)
Fetch(
    meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer
    GIT_TAG ${MESHOPTIMIZER_VERSION}
)

## SPIRV-Reflect
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
Fetch(
    spirvreflect
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
    GIT_TAG ${SPIRVREFLECT_VERSION}
)
