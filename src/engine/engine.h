#pragma once

// STL
#include <algorithm>
#include <array>
#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Vulkan
#include <vulkan/vulkan_raii.hpp>

// KTX
#include <ktx.h>

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// VMA
#include <vk_mem_alloc.h>

// GLM
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>

// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

// EnTT
#include <entt/entt.hpp>

// JoltPhysics
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>

// fastgltf
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

// thread-pool
#include <BS_thread_pool.hpp>

// spdlog
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// cereal
#include <cereal/archives/binary.hpp>

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
constexpr uint32_t MAX_TEXTURES = 16;
const std::string APP_NAME = "GNVEApp";
const std::string ENGINE_NAME = "GNVEngine";
// const std::string MODEL_PATH = "assets/models/square.glb";
const std::string MODEL_PATH = "assets/models/viking_room.glb";
const std::string SHADER_PATH = "shaders/shader.spv";

const std::vector<char const*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        return { vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
                 vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)) };
    }

    bool operator==(const Vertex& other) const { return pos == other.pos && texCoord == other.texCoord; }
};

template <> struct std::hash<Vertex> {
    size_t operator()(Vertex const& vertex) const noexcept
    {
        return (std::hash<glm::vec3>()(vertex.pos) ^ (std::hash<glm::vec2>()(vertex.texCoord) << 1));
    }
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct CameraControls {
    glm::vec3 position = { 2.0f, 2.0f, 2.0f };
    glm::vec3 target = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    float fov = 45.0f;
};

struct Texture {
    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory imageMemory = nullptr;
    vk::raii::ImageView imageView = nullptr;
    vk::Format imageFormat = vk::Format::eUndefined;
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
    vk::raii::Buffer indexBuffer = nullptr;
    vk::raii::DeviceMemory indexBufferMemory = nullptr;
    size_t textureIndex;
};

class GNVEngine
{
  public:
    void run()
    {
        setup_logger();
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

  private:
    UniformBufferObject ubo{};
    CameraControls camera{};

    std::vector<Texture> textureManager;
    uint32_t maxLod = 0;
    vk::raii::Sampler textureSampler = nullptr;
    std::vector<Mesh> meshManager;

    ImGuiIO io;

    GLFWwindow* window = nullptr;
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
    uint32_t queueIndex = ~0;
    vk::raii::Queue queue = nullptr;
    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::raii::ImageView> swapChainImageViews;

    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;

    vk::raii::Image depthImage = nullptr;
    vk::raii::DeviceMemory depthImageMemory = nullptr;
    vk::raii::ImageView depthImageView = nullptr;
    vk::Format depthFormat = vk::Format::eUndefined;

    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    vk::raii::DescriptorPool imGuidescriptorPool = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;

    vk::raii::CommandPool commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;
    uint32_t frameIndex = 0;

    bool framebufferResized = false;

    std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
                                                         vk::KHRSynchronization2ExtensionName,
                                                         vk::KHRCreateRenderpass2ExtensionName };

    void createDescriptorSetLayout();
    void createDepthResources();
    void findDepthFormat();
    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                   vk::FormatFeatureFlags features) const;
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling,
                     vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image,
                     vk::raii::DeviceMemory& imageMemory);
    vk::raii::ImageView createImageView(vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags,
                                        uint32_t mipLevels);
    void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                               uint32_t mipLevels);
    std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands();
    void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;
    void copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);
    void loadModel();
    void createUniformBuffers();
    void createDescriptorSets();
    void updateUniformBuffer(uint32_t currentImage);
    uint32_t addTextureToBindless(vk::raii::DescriptorSet& descriptorSet, Texture& tex, uint32_t slot);

    void createVertexBuffer(Mesh& mesh);
    void createIndexBuffer(Mesh& mesh);

    size_t createTexture(const uint8_t* ktxData, size_t ktxSize);
    void createTextureSampler();

    void setup_logger();
    void initImGui();
    void newImGuiFrame();

    void createDescriptorPools();
    void cleanupSwapChain();
    void recreateSwapChain();
    void createSwapChain();
    void createImageViews();
    void createSyncObjects();

    void initWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createGraphicsPipeline();
    void createCommandPool();
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    void createCommandBuffers();
    void recordCommandBuffer(uint32_t imageIndex);
    void transition_image_layout(vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                                 vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
                                 vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask,
                                 vk::ImageAspectFlags image_aspect_flags);
    void drawFrame();
    [[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
    static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    std::vector<const char*> getRequiredExtensions();
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void*);
    static std::vector<char> readFile(const std::string& filename);
    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                      vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);
    void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
};

class ImGuiSink : public spdlog::sinks::base_sink<std::mutex>
{
  public:
    struct LogEntry {
        std::string msg;
        ImVec4 color;
    };

    std::vector<LogEntry> buffer;
    size_t max_size = 1024;

  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        base_sink<std::mutex>::formatter_->format(msg, formatted);

        if (buffer.size() >= max_size)
            buffer.erase(buffer.begin());

        ImVec4 color{};
        switch (msg.level) {
        case spdlog::level::trace:
            color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        case spdlog::level::debug:
            color = ImVec4(0.2f, 0.6f, 0.8f, 1.0f);
            break;
        case spdlog::level::info:
            color = ImVec4(0.2f, 0.6f, 0.2f, 1.0f);
            break;
        case spdlog::level::warn:
            color = ImVec4(0.8f, 0.6f, 0.0f, 1.0f);
            break;
        case spdlog::level::err:
            color = ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
            break;
        case spdlog::level::critical:
            color = ImVec4(0.4f, 0.0f, 0.0f, 1.0f);
            break;
        default:
            color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        }

        buffer.emplace_back(LogEntry{ fmt::to_string(formatted), color });
    }

    void flush_() override {}
};

struct EngineLog {
    static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> file_sink;
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
    static std::shared_ptr<ImGuiSink> imgui_sink;
    static std::shared_ptr<spdlog::logger> logger;
    static uint32_t maxLogSize;
    static uint32_t maxLogs;
};
