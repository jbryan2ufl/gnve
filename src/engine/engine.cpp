#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "engine/engine.h"

template<typename Archive>
void serialize(
	Archive &archive,
	Position &position)
{
    archive(position.value.x, position.value.y, position.value.z);
}

template<typename Archive>
void serialize(
	Archive &archive,
	Velocity &velocity)
{
    archive(velocity.value.x, velocity.value.y, velocity.value.z);
}

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
		const std::string filename = "test_serial.bin";
		{
			entt::registry registry;
			auto particle = registry.create();

			registry.emplace<Position>(particle, glm::vec3{0.0f, 0.0f, 0.0f});
			registry.emplace<Velocity>(particle, glm::vec3{1.0f, 2.0f, 3.0f});

			try
			{
				std::ofstream ofs{filename, std::ios::binary};

				cereal::BinaryOutputArchive out{ofs};

				entt::snapshot{registry}
					.get<entt::entity>(out)
					.get<Position>(out)
					.get<Velocity>(out);

				registry.view<Position, Velocity>().each([](auto entity, auto &pos, auto &vel) {
				std::cout << "Entity " << int(entt::to_integral(entity))
						<< " pos: " << pos.value.x << ", " << pos.value.y << ", " << pos.value.z
						<< " vel: " << vel.value.x << ", " << vel.value.y << ", " << vel.value.z
						<< "\n";
				});
			}
			catch (const std::exception& e)
			{
				std::cerr << "cereal error: " << e.what() << '\n';
			}
		}

		{
			try
			{
				entt::registry registry;

				std::ifstream ifs{filename, std::ios::binary};

				cereal::BinaryInputArchive in{ifs};

				entt::snapshot_loader{registry}
					.get<entt::entity>(in)
					.get<Position>(in)
					.get<Velocity>(in);
			}
			catch (const std::exception& e)
			{
				std::cerr << "cereal error: " << e.what() << '\n';
			}
		}

		std::cout << "EnTT linked OK\n";
		std::cout << "cereal linked OK\n";
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
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::warn);

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
		file_sink->set_level(spdlog::level::trace);

		spdlog::logger logger("multi_sink", {console_sink, file_sink});
		logger.set_level(spdlog::level::debug);
		logger.warn("this should appear in both console and file");
		logger.info("this message should not appear in the console, only in the file");
	}

}
