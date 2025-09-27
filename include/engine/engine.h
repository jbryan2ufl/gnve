#pragma once

// STL
#include <iostream>
#include <future>
#include <utility>

// GLFW
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Vulkan & Volk
#include <volk.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

// GLM
#include <glm/glm.hpp>

// ImGui
#define IMGUI_IMPL_VULKAN_USE_VOLK
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// EnTT
#include <entt/entt.hpp>

// JoltPhysics
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>

// fastgltf
#include <fastgltf/core.hpp>

// // vk_bootstrap
// #include <VkBootstrap.h>

// VMA
#include <vk_mem_alloc.h>

// thread-pool
#include <BS_thread_pool.hpp>

// zpp::bits
#include <zpp_bits.h>

// // fmt
// #include <fmt/format.h>

// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void testLibraries();
