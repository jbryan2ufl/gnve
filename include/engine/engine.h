#pragma once

// STL
#include <iostream>

// GLFW
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Vulkan & Volk
#include <volk.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

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

void testLibraries();
