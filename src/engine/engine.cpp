#include "engine.h"

static ImGui_ImplVulkanH_Window main_window;

std::shared_ptr<spdlog::sinks::basic_file_sink_mt> EngineLog::file_sink = nullptr;
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> EngineLog::console_sink = nullptr;
std::shared_ptr<ImGuiSink> EngineLog::imgui_sink = nullptr;
std::shared_ptr<spdlog::logger> EngineLog::logger = nullptr;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    EngineLog::logger->error("Vulkan ealidation: {}", pCallbackData->pMessage);
    return VK_FALSE;
}

static void glfw_error_callback(int error, const char* description)
{
    EngineLog::logger->error("GLFW Error {}: {}", error, description);
}

static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    EngineLog::logger->error("VkResult Error: {}", static_cast<int>(err));
    if (err < 0)
        std::abort();
}

static void check_vk_result(vk::Result err)
{
    if (err == vk::Result::eSuccess)
        return;
    EngineLog::logger->error("VkResult Error: {}", static_cast<int>(err));
    if (static_cast<int>(err) < 0)
        std::abort();
}

void Engine::init()
{
    setup_logger();

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        EngineLog::logger->error("Failed to initialize GLFW");
        std::abort();
    }
    vkb::Instance vkb_inst = vkb::InstanceBuilder{}
                                     .set_app_name(name.c_str())
                                     .request_validation_layers()
                                     .use_default_debug_messenger()
                                     .build()
                                     .value();
    instance = vkb_inst.instance;
    debug_messenger = vkb_inst.debug_messenger;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(START_WINDOW_WIDTH, START_WINDOW_HEIGHT, name.c_str(), nullptr, nullptr);
    if (!glfwVulkanSupported()) {
        EngineLog::logger->error("GLFW: Vulkan Not Supported");
        return;
    }

    check_vk_result(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    wd = &main_window;
    wd->Surface = surface;

    vkb::PhysicalDevice vkb_physical_device =
            vkb::PhysicalDeviceSelector{ vkb_inst }.set_surface(surface).select().value();
    physical_device = vkb_physical_device.physical_device;
    vkb::Device vkb_device = vkb::DeviceBuilder{ vkb_physical_device }.build().value();
    device = vkb_device.device;
    queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family, wd->Surface, &res);
    if (res != VK_TRUE) {
        EngineLog::logger->error("Error no WSI support on physical device");
        std::abort();
    }
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physical_device, wd->Surface, requestSurfaceImageFormat,
                                                              (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
                                                              requestSurfaceColorSpace);

    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
                                         VK_PRESENT_MODE_FIFO_KHR };
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physical_device, wd->Surface, &present_modes[0],
                                                          IM_ARRAYSIZE(present_modes));

    vk::DescriptorPoolSize pool_size{};
    pool_size.type = vk::DescriptorType::eCombinedImageSampler;
    pool_size.descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;

    vk::DescriptorPoolCreateInfo pool_info{};
    pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    pool_info.maxSets = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;

    check_vk_result(device.createDescriptorPool(&pool_info, nullptr, &descriptor_pool));

    ImGui_ImplVulkanH_CreateOrResizeWindow(vkb_inst.instance, vkb_physical_device.physical_device, device, wd,
                                           queue_family, nullptr, START_WINDOW_WIDTH, START_WINDOW_HEIGHT,
                                           min_image_count, 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = instance;
    info.PhysicalDevice = vkb_physical_device.physical_device;
    info.Device = device;
    info.QueueFamily = queue_family;
    info.Queue = queue;
    info.PipelineCache = VK_NULL_HANDLE;
    info.DescriptorPool = descriptor_pool;
    info.MinImageCount = min_image_count;
    info.ImageCount = wd->ImageCount;
    info.Allocator = nullptr;
    info.PipelineInfoMain.RenderPass = wd->RenderPass;
    info.PipelineInfoMain.Subpass = 0;
    info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&info);

    clear_value.setColor(vk::ClearColorValue().setFloat32({ 0.f, 0.f, 0.f, 1.f }));

    EngineLog::logger->info("Engine::init() completed");
}

void Engine::setup_logger()
{
    EngineLog::console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    EngineLog::console_sink->set_level(spdlog::level::warn);

    EngineLog::file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
    EngineLog::file_sink->set_level(spdlog::level::trace);

    EngineLog::imgui_sink = std::make_shared<ImGuiSink>();
    EngineLog::imgui_sink->set_level(spdlog::level::trace);

    EngineLog::logger = std::make_shared<spdlog::logger>(
            name.c_str(),
            spdlog::sinks_init_list{ EngineLog::console_sink, EngineLog::file_sink, EngineLog::imgui_sink });

    EngineLog::logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(EngineLog::logger);
    spdlog::flush_on(spdlog::level::warn);

    EngineLog::logger->info("Loggers setup");
}

void Engine::run()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int32_t fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 && (rebuild || wd->Width != fb_width || wd->Height != fb_height)) {
            EngineLog::logger->info("Framebuffer resized w: {} h: {}", fb_width, fb_height);
            ImGui_ImplVulkan_SetMinImageCount(min_image_count);
            ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physical_device, device, wd, queue_family, nullptr,
                                                   fb_width, fb_height, min_image_count, 0);
            wd->FrameIndex = 0;
            rebuild = false;
        }
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin(name.c_str());

        if (EngineLog::imgui_sink != nullptr) {
            for (auto& line : EngineLog::imgui_sink->buffer)
                ImGui::TextUnformatted(line.c_str());
        }

        ImGui::End();
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        vk::Semaphore image_acquired = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        vk::Semaphore render_complete = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        vk::Result res =
                device.acquireNextImageKHR(wd->Swapchain, UINT64_MAX, image_acquired, VK_NULL_HANDLE, &wd->FrameIndex);
        if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR)
            rebuild = true;
        if (res == vk::Result::eErrorOutOfDateKHR)
            continue;
        if (res != vk::Result::eSuboptimalKHR)
            check_vk_result(res);
        ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
        check_vk_result(vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX));
        check_vk_result(vkResetFences(device, 1, &fd->Fence));
        check_vk_result(vkResetCommandPool(device, fd->CommandPool, 0));
        vk::CommandBuffer command_buffer{ fd->CommandBuffer };

        {
            vk::CommandBufferBeginInfo info{};
            info.sType = vk::StructureType::eCommandBufferBeginInfo;
            info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            command_buffer.begin(info);
        }

        {
            vk::RenderPassBeginInfo info{};
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.offset = vk::Offset2D{ 0, 0 };
            info.renderArea.extent =
                    vk::Extent2D{ static_cast<uint32_t>(wd->Width), static_cast<uint32_t>(wd->Height) };
            info.clearValueCount = 1;
            info.pClearValues = &clear_value;
            command_buffer.beginRenderPass(info, vk::SubpassContents::eInline);
        }

        ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);
        command_buffer.endRenderPass();

        {
            vk::PipelineStageFlags wait_stage{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
            vk::SubmitInfo info{};
            info.sType = vk::StructureType::eSubmitInfo;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &command_buffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete;

            check_vk_result(vkEndCommandBuffer(command_buffer));
            queue.submit({ info }, fd->Fence);
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        {
            if (rebuild)
                continue;
            vk::SwapchainKHR raw_swapchain = static_cast<vk::SwapchainKHR>(wd->Swapchain);
            vk::PresentInfoKHR info{};
            info.sType = vk::StructureType::ePresentInfoKHR;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &render_complete;
            info.swapchainCount = 1;
            info.pSwapchains = &raw_swapchain;
            info.pImageIndices = &wd->FrameIndex;
            check_vk_result(queue.presentKHR(info));
            wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;
        }
    }
}

void Engine::clean_up()
{
    check_vk_result(vkDeviceWaitIdle(device));
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (wd) {
        ImGui_ImplVulkanH_DestroyWindow(instance, device, wd, nullptr);
    }

    device.destroyDescriptorPool(descriptor_pool, nullptr);

    device.destroy();
    vkb::destroy_debug_utils_messenger(instance, debug_messenger);
    instance.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void testLibraries()
{
    {
        const std::string filename = "test_serial.bin";
        {
            entt::registry registry;
            auto particle = registry.create();

            registry.emplace<Position>(particle, glm::vec3{ 0.0f, 0.0f, 0.0f });
            registry.emplace<Velocity>(particle, glm::vec3{ 1.0f, 2.0f, 3.0f });

            try {
                std::ofstream ofs{ filename, std::ios::binary };

                cereal::BinaryOutputArchive out{ ofs };

                entt::snapshot{ registry }.get<entt::entity>(out).get<Position>(out).get<Velocity>(out);

                registry.view<Position, Velocity>().each([](auto entity, auto& pos, auto& vel) {
                    std::cout << "Entity " << int(entt::to_integral(entity)) << " pos: " << pos.value.x << ", "
                              << pos.value.y << ", " << pos.value.z << " vel: " << vel.value.x << ", " << vel.value.y
                              << ", " << vel.value.z << "\n";
                });
            } catch (const std::exception& e) {
                std::cerr << "cereal error: " << e.what() << '\n';
            }
        }

        {
            try {
                entt::registry registry;

                std::ifstream ifs{ filename, std::ios::binary };

                cereal::BinaryInputArchive in{ ifs };

                entt::snapshot_loader{ registry }.get<entt::entity>(in).get<Position>(in).get<Velocity>(in);
            } catch (const std::exception& e) {
                std::cerr << "cereal error: " << e.what() << '\n';
            }
        }

        std::cout << "EnTT linked OK\n";
        std::cout << "cereal linked OK\n";
    }
}

template <typename Archive> void serialize(Archive& archive, Position& position)
{
    archive(position.value.x, position.value.y, position.value.z);
}

template <typename Archive> void serialize(Archive& archive, Velocity& velocity)
{
    archive(velocity.value.x, velocity.value.y, velocity.value.z);
}
