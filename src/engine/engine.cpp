#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "engine/engine.h"

void testLibraries()
{
	{
		if (glfwInit() == GLFW_TRUE) {
			std::cout << "GLFW linked OK\n";
			glfwTerminate();
		}
	}

	{
		if (volkInitialize() != VK_SUCCESS)
		{
        	std::cout << "volkInitialize failed\n";
        	return;
    	}
		else
		{
        	std::cout << "volkInitialize succeeded\n";
    	}
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
	
	// {
	// 	vkb::InstanceBuilder i;
	// 	std::cout << "vk_bootstrap linked OK\n";
	// }

	{
		VkAllocationCallbacks* allocationCallbacks = nullptr;
        VmaAllocatorCreateInfo allocatorInfo = {};
        VmaAllocator allocator{};
        std::cout << "VulkanMemoryAllocator (VMA) headers OK\n";
	}

	{
        BS::thread_pool pool;
        std::future<int> future = pool.submit_task([]{return 42;});
        std::cout << future.get() << " thread-pool linked OK\n";
    }

	{
		using person = std::pair<std::string, int>;
		auto [data, in, out] = zpp::bits::data_in_out();
		out(person{"Person1", 25}, person{"Person2", 35});
		std::cout << "zpp bits linked OK\n";
	}

	{
		auto console = spdlog::stdout_color_mt("console");

		std::string msg = fmt::format("\033[1;32mHello, {}! The answer is {}\033[0m", "World", 42);

		console->info(msg);

		spdlog::drop("console");

		std::cout << "fmt and spdlog linked OK\n";
	}
}

void configureLog()
{
	auto console{spdlog::stdout_color_mt("console")};
	auto sink{std::make_shared<spdlog::sinks::stderr_color_sink_mt>()};
	sink->set_color(spdlog::level::warn, );

	auto file_sink{std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt", true)};
}