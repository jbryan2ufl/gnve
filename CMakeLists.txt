set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.21)
project(main LANGUAGES CXX)

set(GLFW_VERSION "3.4")
set(VOLK_VERSION "1.4.304")
set(VMA_VERSION "v3.3.0")
# set(VKBOOTSTRAP_VERSION "v1.4.315")
set(GLFW_VERSION "3.4")
set(GLM_VERSION "1.0.1")
set(FASTGLTF_VERSION "v0.8.0")
set(THREADPOOL_VERSION "v5.0.0")
set(STB_VERSION "802cd45")
set(JOLTPHYSICS_VERSION "v5.3.0")
set(ENTT_VERSION "v3.15.0")
set(ZPPBITS_VERSION "v4.5")
set(IMGUI_VERSION "v1.91.9b-docking")
# set(FMT_VERSION "11.2.0")
set(SPDLOG_VERSION "v1.15.3")

include(FetchContent)

# VULKAN
find_package(Vulkan 1.4.304.0 REQUIRED)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(Vulkan REQUIRED COMPONENTS glslangValidator)
endif()

# GLFW
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG ${GLFW_VERSION}
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG ${GLM_VERSION}
)
FetchContent_MakeAvailable(glm)

# ImGui
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG ${IMGUI_VERSION}
)
FetchContent_MakeAvailable(imgui)

# EnTT
FetchContent_Declare(entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG ${ENTT_VERSION}
)
FetchContent_MakeAvailable(entt)

# JoltPhysics
set(TARGET_UNIT_TESTS OFF)
set(TARGET_HELLO_WORLD OFF)
set(TARGET_PERFORMANCE_TEST OFF)
set(TARGET_SAMPLES OFF)
FetchContent_Declare(jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics
    GIT_TAG ${JOLTPHYSICS_VERSION}
    SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(jolt)

# fastgltf
FetchContent_Declare(fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf
    GIT_TAG ${FASTGLTF_VERSION}
)
FetchContent_MakeAvailable(fastgltf)

# volk
FetchContent_Declare(volk
    GIT_REPOSITORY https://github.com/zeux/volk
    GIT_TAG ${VOLK_VERSION}
)
FetchContent_MakeAvailable(volk)

# # vk_bootstrap
# FetchContent_Declare(
#     vk_bootstrap
#     GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
#     GIT_TAG ${VKBOOTSTRAP_VERSION}
# )
# FetchContent_MakeAvailable(vk_bootstrap)

# VMA
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG ${VMA_VERSION}
)
FetchContent_MakeAvailable(vma)

# thread-pool
FetchContent_Declare(
    threadpool
    GIT_REPOSITORY https://github.com/bshoshany/thread-pool
    GIT_TAG ${THREADPOOL_VERSION}
)
FetchContent_MakeAvailable(threadpool)

# stb
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG ${STB_VERSION}
)
FetchContent_MakeAvailable(stb)

# zpp::bits
FetchContent_Declare(
    zppbits
    GIT_REPOSITORY https://github.com/eyalz800/zpp_bits
    GIT_TAG ${ZPPBITS_VERSION}
)
FetchContent_MakeAvailable(zppbits)

# # {fmt}
# FetchContent_Declare(
#     fmt
#     GIT_REPOSITORY https://github.com/fmtlib/fmt
#     GIT_TAG ${FMT_VERSION}
# )
# FetchContent_MakeAvailable(fmt)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG ${SPDLOG_VERSION}
)
FetchContent_MakeAvailable(spdlog)

# Sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(main ${SOURCES})

# ImGui sources
target_sources(main PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# Include directories
target_include_directories(main PRIVATE
    include
    ${volk_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${entt_SOURCE_DIR}/single_include
    ${jolt_SOURCE_DIR}/..
	${fastgltf_SOURCE_DIR}/include
    ${threadpool_SOURCE_DIR}/include
    ${zppbits_SOURCE_DIR}
)

# Link libraries
target_link_libraries(main PRIVATE
    Vulkan::Vulkan
    volk::volk
    glfw
    glm::glm
	fastgltf
	Jolt
    # vk-bootstrap::vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
    # fmt::fmt
    spdlog::spdlog
)

# Compiler settings
target_compile_features(main PRIVATE cxx_std_23)
set_target_properties(main PROPERTIES
    CXX_EXTENSIONS OFF
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(main PRIVATE -g -O0)
endif()
