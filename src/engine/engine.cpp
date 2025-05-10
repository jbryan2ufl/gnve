#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "engine/engine.h"

void testLibraries()
{
	if (glfwInit() == GLFW_TRUE) {
		std::cout << "GLFW linked OK\n";
		glfwTerminate();
	}

	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		std::cout << "Volk/Vulkan headers OK\n";
	}

	{
		vk::ApplicationInfo appInfo;
		std::cout << "Vulkan-HPP headers OK\n";
	}

	{
		glm::vec3 test(1.0f);
		std::cout << "GLM linked OK\n";
	}

	{
		entt::registry registry;
		std::cout << "EnTT linked OK\n";
	}

	{
		JPH::RegisterDefaultAllocator();
		std::cout << "JoltPhysics linked OK\n";
	}

	{
		fastgltf::Parser parser;
		std::cout << "fastgltf linked OK\n";
	}

	{
		ImGuiContext* ctx = ImGui::CreateContext();
		std::cout << "ImGui linked OK\n";
		ImGui::DestroyContext(ctx);
	}
}