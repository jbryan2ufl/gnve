file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
add_library(GNVEngine STATIC ${SOURCES})

set_target_properties(GNVEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "GNVEngine"
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_sources(GNVEngine
PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(GNVEngine
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${entt_SOURCE_DIR}/single_include
    ${jolt_SOURCE_DIR}/..
    ${fastgltf_SOURCE_DIR}/include
    ${threadpool_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${cereal_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}
    ${ktx_SOURCE_DIR}/include
)

target_link_libraries(GNVEngine
PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
    GPUOpen::VulkanMemoryAllocator
    spdlog::spdlog
    fastgltf
    Jolt
    ktx
)
