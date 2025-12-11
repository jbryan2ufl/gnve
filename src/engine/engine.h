#pragma once

// STL
#include "glm/fwd.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <future>
#include <utility>
#include <random>

// Vulkan
#include <vulkan/vulkan.hpp>

// GLFW
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vk-boostrap
#include <VkBootstrap.h>

// VMA
#include <vk_mem_alloc.h>

// GLM
#include <glm/glm.hpp>

// ImGui
// #define IMGUI_IMPL_VULKAN_USE_VOLK
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

// thread-pool
#include <BS_thread_pool.hpp>

// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/color.h>

// cereal
#include <cereal/archives/binary.hpp>

struct Position {
    glm::vec3 value;
};

struct Velocity {
    glm::vec3 value;
};

void testLibraries();

class ImGuiSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
    std::vector<std::string> buffer;
    size_t max_size = 1024;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);

        if (buffer.size() >= max_size)
            buffer.erase(buffer.begin());

        buffer.emplace_back(fmt::to_string(formatted));
    }

    void flush_() override
    {
    }
};

#define START_WINDOW_WIDTH 1920
#define START_WINDOW_HEIGHT 1920

class Engine
{
public:
    Engine() {};
    void init();
    void setup_logger();
    void run();
    void clean_up();

private:
    int32_t width = 1920;
    int32_t height = 1080;
    std::string name = "GNVE";
    vk::Device device;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vk::Instance instance;
    vk::Queue queue;
    uint32_t queue_family;
    GLFWwindow* window;
    ImGui_ImplVulkanH_Window* wd;
    uint32_t min_image_count = 2;
    ImGuiIO io;
    bool rebuild = false;
    vk::ClearValue clear_value;
    vk::PhysicalDevice physical_device;
    vk::DescriptorPool descriptor_pool;
    vk::DebugUtilsMessengerEXT debug_messenger;
};

class EngineLog
{
public:
    static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;
    static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
    static std::shared_ptr<ImGuiSink> imgui_sink;
    static std::shared_ptr<spdlog::logger> logger;
};
