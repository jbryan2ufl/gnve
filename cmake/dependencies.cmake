set(CMAKE_EXTERNALPROJECT_DOWNLOAD_PARALLEL 8)
set(FETCHCONTENT_DOWNLOAD_PARALLEL 8)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

include(FetchContent)

set(STRIP_GIT ON CACHE BOOL "" FORCE)

macro(Fetch name)
    FetchContent_Declare(${name}
        ${ARGN}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        GIT_SUBMODULES ""
    )
    FetchContent_MakeAvailable(${name})
    if(STRIP_GIT)
        file(REMOVE_RECURSE "${${name}_SOURCE_DIR}/.git")
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
    GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579
)

# GLM
Fetch(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 8d1fd52e5ab5590e2c81768ace50c72bae28f2ed
)

# ImGui
Fetch(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG 3912b3d9a9c1b3f17431aebafd86d2f40ee6e59c
)

# EnTT
Fetch(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG b4e58bdd364ad72246c123a0c28538eab3252672
)

# JoltPhysics
set(ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
Fetch(
    jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics
    GIT_TAG 23dadd0e603f1b321142d4c74df07fce85064989
    SOURCE_SUBDIR "Build"
)

# fastgltf
Fetch(
    fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf
    GIT_TAG 0d1b67a28c4950ea2deb796702006dcbe31e02b3
    EXCLUDE_FROM_ALL
)

# VMA
Fetch(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG 1d8f600fd424278486eade7ed3e877c99f0846b1
)

# thread-pool
Fetch(
    threadpool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool
    GIT_TAG aa3fbfbe80762fe3ac90e2bf05e153b92536277a
)

# # stb
# Fetch(
#     stb
#     GIT_REPOSITORY https://github.com/nothings/stb
#     GIT_TAG f1c79c02822848a9bed4315b12c8c8f3761e1296
# )

# spdlog
Fetch(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG 486b55554f11c9cccc913e11a87085b2a91f706f
)

# cereal
Fetch(
    cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal
    GIT_TAG ebef1e929807629befafbb2918ea1a08c7194554
    SOURCE_SUBDIR ""
)

# KTX
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_JS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
Fetch(
    ktx
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software
    GIT_TAG 4d6fc70eaf62ad0558e63e8d97eb9766118327a6
    EXCLUDE_FROM_ALL
)

# basisu
Fetch(
    basisu
    GIT_REPOSITORY https://github.com/BinomialLLC/basis_universal
    GIT_TAG 5c511882f1fdacfac798e83b5102f2f782d1de2f
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
    GIT_TAG 73583c335e541c139821d0de2bf5f12960a04941
)

# SPIRV-Reflect
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
Fetch(
    spirvreflect
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
    GIT_TAG ef913b3ab3da1becca3cf46b15a10667c67bebe5
)
