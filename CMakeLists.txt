set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.21)
project(main LANGUAGES CXX)

include(FetchContent)

# VULKAN
find_package(Vulkan REQUIRED)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(Vulkan REQUIRED COMPONENTS glslangValidator)
endif()

# GLFW
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

# ImGui
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# EnTT
FetchContent_Declare(entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
)
FetchContent_MakeAvailable(entt)

# JoltPhysics
set(TARGET_UNIT_TESTS OFF)
set(TARGET_HELLO_WORLD OFF)
set(TARGET_PERFORMANCE_TEST OFF)
set(TARGET_SAMPLES OFF)
FetchContent_Declare(jolt
    GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
    SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(jolt)

# fastgltf
FetchContent_Declare(fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf.git
    GIT_TAG main
)
FetchContent_MakeAvailable(fastgltf)

# volk
FetchContent_Declare(volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
)
FetchContent_MakeAvailable(volk)

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
)

# Link libraries
target_link_libraries(main PRIVATE
    Vulkan::Vulkan
    volk::volk
    glfw
    glm::glm
	fastgltf
	Jolt
)

# Compiler settings
target_compile_features(main PRIVATE cxx_std_23)
set_target_properties(main PROPERTIES
    CXX_EXTENSIONS OFF
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(main PRIVATE -g -O0)
endif()
