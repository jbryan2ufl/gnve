#include <engine.h>

std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> EngineLog::file_sink = nullptr;
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> EngineLog::console_sink = nullptr;
std::shared_ptr<ImGuiSink> EngineLog::imgui_sink = nullptr;
std::shared_ptr<spdlog::logger> EngineLog::logger = nullptr;
uint32_t EngineLog::maxLogSize = 1024 * 1024 * 10;
uint32_t EngineLog::maxLogs = 5;

void GNVEngine::setup_logger()
{
    EngineLog::console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    EngineLog::console_sink->set_level(spdlog::level::trace);

    EngineLog::file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/engine.log", EngineLog::maxLogSize, EngineLog::maxLogs);
    EngineLog::file_sink->set_level(spdlog::level::trace);

    EngineLog::imgui_sink = std::make_shared<ImGuiSink>();
    EngineLog::imgui_sink->set_level(spdlog::level::trace);

    EngineLog::logger = std::make_shared<spdlog::logger>(
        ENGINE_NAME, spdlog::sinks_init_list{ EngineLog::console_sink, EngineLog::file_sink, EngineLog::imgui_sink });
    EngineLog::logger->set_level(spdlog::level::trace);

    spdlog::set_default_logger(EngineLog::logger);
    spdlog::flush_every(std::chrono::seconds(5));

    EngineLog::logger->trace("Loggers setup.");
    EngineLog::logger->trace("Test");
    EngineLog::logger->debug("Test");
    EngineLog::logger->info("Test");
    EngineLog::logger->warn("Test");
    EngineLog::logger->error("Test");
    EngineLog::logger->critical("Test");
}

void GNVEngine::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, APP_NAME.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void GNVEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<GNVEngine*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void GNVEngine::initVulkan()
{
    EngineLog::logger->trace("createInstance()");
    createInstance();
    EngineLog::logger->trace("setupDebugMessenger()");
    setupDebugMessenger();
    EngineLog::logger->trace("createSurface()");
    createSurface();
    EngineLog::logger->trace("pickPhysicalDevice()");
    pickPhysicalDevice();
    EngineLog::logger->trace("createLogicalDevice()");
    createLogicalDevice();
    EngineLog::logger->trace("createSwapChain()");
    createSwapChain();
    EngineLog::logger->trace("createImageViews()");
    createImageViews();
    EngineLog::logger->trace("createDescriptorSetLayout()");
    createDescriptorSetLayout();
    EngineLog::logger->trace("createGraphicsPipeline()");
    createGraphicsPipeline();
    EngineLog::logger->trace("createCommandPool()");
    createCommandPool();
    EngineLog::logger->trace("createDepthResources()");
    createDepthResources();
    EngineLog::logger->trace("createTextureSampler()");
    createTextureSampler();
    EngineLog::logger->trace("createUniformBuffers()");
    createUniformBuffers();
    EngineLog::logger->trace("createDescriptorPools()");
    createDescriptorPools();
    EngineLog::logger->trace("createDescriptorSets()");
    createDescriptorSets();
    EngineLog::logger->trace("createCommandBuffers()");
    createCommandBuffers();
    EngineLog::logger->trace("createSyncObjects()");
    createSyncObjects();
    EngineLog::logger->trace("initImGui()");
    initImGui();
    EngineLog::logger->trace("loadModel()");
    loadModel();
}

void GNVEngine::mainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    device.waitIdle();
}

void GNVEngine::cleanupSwapChain()
{
    swapChainImageViews.clear();
    swapChain = nullptr;
}

void GNVEngine::cleanup()
{
    device.waitIdle();

    meshManager.clear();
    textureManager.clear();
    textureSampler.clear();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void GNVEngine::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();

    cleanupSwapChain();
    createSwapChain();
    createImageViews();
    createDepthResources();
}

void GNVEngine::createInstance()
{
    vk::ApplicationInfo appInfo{};
    appInfo.setPApplicationName(APP_NAME.c_str())
        .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName(ENGINE_NAME.c_str())
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion(vk::ApiVersion14);

    // Get the required layers
    std::vector<char const*> requiredLayers;
    if (enableValidationLayers) {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = context.enumerateInstanceLayerProperties();
    for (auto const& requiredLayer : requiredLayers) {
        if (std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty) {
                return strcmp(layerProperty.layerName, requiredLayer) == 0;
            })) {
            throw std::runtime_error("Required layer not supported: " + std::string(requiredLayer));
        }
    }

    // Get the required extensions.
    auto requiredExtensions = getRequiredExtensions();

    // Check if the required extensions are supported by the Vulkan implementation.
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for (auto const& requiredExtension : requiredExtensions) {
        if (std::ranges::none_of(extensionProperties, [requiredExtension](auto const& extensionProperty) {
                return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
            })) {
            throw std::runtime_error("Required extension not supported: " + std::string(requiredExtension));
        }
    }

    vk::InstanceCreateInfo createInfo{};
    createInfo.setPApplicationInfo(&appInfo)
        .setPEnabledLayerNames(requiredLayers)
        .setPEnabledExtensionNames(requiredExtensions);
    instance = vk::raii::Instance(context, createInfo);
}

void GNVEngine::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{ vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{ vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
    debugUtilsMessengerCreateInfoEXT.setMessageSeverity(severityFlags)
        .setMessageType(messageTypeFlags)
        .setPfnUserCallback(&debugCallback);
    debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void GNVEngine::createSurface()
{
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
        throw std::runtime_error("failed to create window surface!");
    }
    surface = vk::raii::SurfaceKHR(instance, _surface);
}

void GNVEngine::pickPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    const auto devIter = std::ranges::find_if(devices, [&](auto const& device) {
        // Check if the device supports the Vulkan 1.3 API version
        bool supportsVulkan1_3 = device.getProperties().apiVersion >= VK_API_VERSION_1_3;

        // Check if any of the queue families support graphics operations
        auto queueFamilies = device.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of(
            queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

        // Check if all required device extensions are available
        auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions = std::ranges::all_of(
            requiredDeviceExtension, [&availableDeviceExtensions](auto const& requiredDeviceExtension) {
                return std::ranges::any_of(
                    availableDeviceExtensions, [requiredDeviceExtension](auto const& availableDeviceExtension) {
                        return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
                    });
            });

        auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                                                     vk::PhysicalDeviceVulkan13Features,
                                                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures =
            features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
    });
    if (devIter != devices.end()) {
        physicalDevice = *devIter;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void GNVEngine::createLogicalDevice()
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports both graphics and present
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)) {
            // found a queue family that supports both graphics and present
            queueIndex = qfpIndex;
            break;
        }
    }
    if (queueIndex == ~0) {
        throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
    }

    // query for required features (Vulkan 1.1 and 1.3)
    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
                       vk::PhysicalDeviceDescriptorIndexingFeatures>
        featureChain{};
    featureChain.get<vk::PhysicalDeviceFeatures2>().features.setSamplerAnisotropy(VK_TRUE);
    featureChain.get<vk::PhysicalDeviceVulkan11Features>().setShaderDrawParameters(VK_TRUE);
    featureChain.get<vk::PhysicalDeviceVulkan13Features>().setSynchronization2(VK_TRUE).setDynamicRendering(VK_TRUE);
    featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().setExtendedDynamicState(VK_TRUE);
    featureChain.get<vk::PhysicalDeviceDescriptorIndexingFeatures>()
        .setRuntimeDescriptorArray(VK_TRUE)
        .setDescriptorBindingPartiallyBound(VK_TRUE)
        .setShaderSampledImageArrayNonUniformIndexing(VK_TRUE)
        .setDescriptorBindingVariableDescriptorCount(VK_TRUE)
        .setDescriptorBindingUpdateUnusedWhilePending(VK_TRUE)
        .setDescriptorBindingUniformBufferUpdateAfterBind(VK_TRUE)
        .setDescriptorBindingSampledImageUpdateAfterBind(VK_TRUE);

    // create a Device
    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.setQueueFamilyIndex(queueIndex).setQueueCount(1).setPQueuePriorities(&queuePriority);

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.setPNext(&featureChain.get<vk::PhysicalDeviceFeatures2>())
        .setQueueCreateInfoCount(1)
        .setPQueueCreateInfos(&deviceQueueCreateInfo)
        .setEnabledExtensionCount(static_cast<uint32_t>(requiredDeviceExtension.size()))
        .setPpEnabledExtensionNames(requiredDeviceExtension.data());

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    queue = vk::raii::Queue(device, queueIndex, 0);
}

void GNVEngine::createSwapChain()
{
    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    swapChainExtent = chooseSwapExtent(surfaceCapabilities);
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
    vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.setSurface(surface)
        .setMinImageCount(chooseSwapMinImageCount(surfaceCapabilities))
        .setImageFormat(swapChainSurfaceFormat.format)
        .setImageColorSpace(swapChainSurfaceFormat.colorSpace)
        .setImageExtent(swapChainExtent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(vk::SharingMode::eExclusive)
        .setPreTransform(surfaceCapabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(chooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(surface)))
        .setClipped(VK_TRUE);

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
}

void GNVEngine::createImageViews()
{
    assert(swapChainImageViews.empty());

    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.setViewType(vk::ImageViewType::e2D)
        .setFormat(swapChainSurfaceFormat.format)
        .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    for (auto& image : swapChainImages) {
        imageViewCreateInfo.image = image;
        swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }
}

void GNVEngine::createDescriptorPools()
{
    vk::DescriptorPoolSize imGuipoolSize{};
    imGuipoolSize.setType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE);

    vk::DescriptorPoolCreateInfo imGuipoolInfo{};
    imGuipoolInfo
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                  vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
        .setMaxSets(IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE)
        .setPoolSizeCount(1)
        .setPPoolSizes(&imGuipoolSize);

    imGuidescriptorPool = vk::raii::DescriptorPool{ device, imGuipoolInfo };

    std::array poolSize{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
                         vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT) };
    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
                  vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
        .setMaxSets(MAX_FRAMES_IN_FLIGHT)
        .setPoolSizeCount(static_cast<uint32_t>(poolSize.size()))
        .setPPoolSizes(poolSize.data());
    descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
}

void GNVEngine::initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);

    ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
    pipelineInfo.RenderPass = VK_NULL_HANDLE;
    pipelineInfo.Subpass = 0;
    pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    pipelineInfo.PipelineRenderingCreateInfo = {};
    pipelineInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    VkFormat colorFormat = static_cast<VkFormat>(swapChainSurfaceFormat.format);
    pipelineInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
    pipelineInfo.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipelineInfo.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = *instance;
    info.PhysicalDevice = *physicalDevice;
    info.Device = *device;
    info.QueueFamily = queueIndex;
    info.Queue = *queue;
    info.PipelineCache = VK_NULL_HANDLE;
    info.DescriptorPool = *imGuidescriptorPool;
    info.MinImageCount = chooseSwapMinImageCount(surfaceCapabilities);
    info.ImageCount = swapChainImages.size();
    info.PipelineInfoMain = pipelineInfo;
    info.UseDynamicRendering = true;
    ImGui_ImplVulkan_Init(&info);
}

void GNVEngine::createGraphicsPipeline()
{
    vk::raii::ShaderModule shaderModule = createShaderModule(readFile(SHADER_PATH));

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex).setModule(shaderModule).setPName("vertMain");
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment).setModule(shaderModule).setPName("fragMain");
    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.setVertexBindingDescriptionCount(1)
        .setPVertexBindingDescriptions(&bindingDescription)
        .setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
        .setPVertexAttributeDescriptions(attributeDescriptions.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);

    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.setViewportCount(1).setScissorCount(1);

    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.setDepthClampEnable(vk::False)
        .setRasterizerDiscardEnable(vk::False)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setCullMode(vk::CullModeFlagBits::eNone)
        // .setCullMode(vk::CullModeFlagBits::eBack)
        // .setFrontFace(vk::FrontFace::eClockwise)
        .setDepthBiasEnable(vk::False)
        .setLineWidth(1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1).setSampleShadingEnable(vk::False);

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.setDepthTestEnable(vk::True)
        .setDepthWriteEnable(vk::True)
        // depthStencil.setDepthTestEnable(vk::False)
        //     .setDepthWriteEnable(vk::False)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(vk::False)
        .setStencilTestEnable(vk::False);

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.setBlendEnable(vk::False).setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.setLogicOpEnable(vk::False)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachmentCount(1)
        .setPAttachments(&colorBlendAttachment);

    std::vector dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
        .setPDynamicStates(dynamicStates.data());

    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eFragment).setOffset(0).setSize(sizeof(uint32_t));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayoutCount(1)
        .setPushConstantRangeCount(1)
        .setPPushConstantRanges(&pushConstantRange)
        .setPSetLayouts(&*descriptorSetLayout);

    pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

    findDepthFormat();
    EngineLog::logger->debug("Depth format {}", vk::to_string(depthFormat));
    vk::PipelineRenderingCreateInfo pipelineRenderingInfo{};
    pipelineRenderingInfo.setColorAttachmentCount(1)
        .setPColorAttachmentFormats(&swapChainSurfaceFormat.format)
        .setDepthAttachmentFormat(depthFormat);

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.setStageCount(2)
        .setPStages(shaderStages)
        .setPVertexInputState(&vertexInputInfo)
        .setPInputAssemblyState(&inputAssembly)
        .setPViewportState(&viewportState)
        .setPRasterizationState(&rasterizer)
        .setPMultisampleState(&multisampling)
        .setPDepthStencilState(&depthStencil)
        .setPColorBlendState(&colorBlending)
        .setPDynamicState(&dynamicState)
        .setLayout(pipelineLayout)
        .setRenderPass(nullptr)
        .setPNext(&pipelineRenderingInfo);

    graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfo);
}

void GNVEngine::createCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer).setQueueFamilyIndex(queueIndex);
    commandPool = vk::raii::CommandPool(device, poolInfo);
}

void GNVEngine::createVertexBuffer(Mesh& mesh)
{
    vk::DeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();
    vk::raii::Buffer stagingBuffer{ nullptr };
    vk::raii::DeviceMemory stagingBufferMemory{ nullptr };
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
                 stagingBufferMemory);
    void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, mesh.vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    createBuffer(bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, mesh.vertexBuffer, mesh.vertexBufferMemory);
    copyBuffer(stagingBuffer, mesh.vertexBuffer, bufferSize);
}

void GNVEngine::createIndexBuffer(Mesh& mesh)
{
    vk::DeviceSize bufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();

    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
                 stagingBufferMemory);

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, mesh.indices.data(), (size_t)bufferSize);
    stagingBufferMemory.unmapMemory();

    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, mesh.indexBuffer, mesh.indexBufferMemory);

    copyBuffer(stagingBuffer, mesh.indexBuffer, bufferSize);
}

uint32_t GNVEngine::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void GNVEngine::createCommandBuffers()
{
    commandBuffers.clear();
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(MAX_FRAMES_IN_FLIGHT);
    commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}

void GNVEngine::recordCommandBuffer(uint32_t imageIndex)
{
    auto& commandBuffer = commandBuffers[frameIndex];
    commandBuffer.begin({});
    // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
    transition_image_layout(
        swapChainImages[imageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
        {}, // srcAccessMask (no need to wait for previous operations)
        vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageAspectFlagBits::eColor);
    transition_image_layout(
        *depthImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth);

    vk::ClearValue clearColor = vk::ClearColorValue(0.2f, 0.2f, 0.2f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo{};
    attachmentInfo.setImageView(swapChainImageViews[imageIndex])
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(clearColor);

    vk::ClearValue clearDepth = vk::ClearDepthStencilValue{ 1.0f, 0 };
    vk::RenderingAttachmentInfo depthAttachmentInfo{};
    depthAttachmentInfo.setImageView(*depthImageView)
        .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setClearValue(clearDepth);

    vk::Rect2D renderArea{};
    renderArea.setOffset({ 0, 0 }).setExtent(swapChainExtent);
    vk::RenderingInfo renderingInfo{};
    renderingInfo.setRenderArea(renderArea)
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&attachmentInfo)
        .setPDepthAttachment(&depthAttachmentInfo);

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                                              static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(
        0, vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D{ static_cast<uint32_t>(swapChainExtent.width),
                                                        static_cast<uint32_t>(swapChainExtent.height) }));
    for (auto& mesh : meshManager) {
        commandBuffer.bindVertexBuffers(0, *mesh.vertexBuffer, { 0 });
        // commandBuffer.bindIndexBuffer(*mesh.indexBuffer, 0,
        //                               vk::IndexTypeValue<decltype(mesh.indices)::value_type>::value);
        commandBuffer.bindIndexBuffer(*mesh.indexBuffer, 0, vk::IndexType::eUint32);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0,
                                         *descriptorSets[frameIndex], nullptr);
        commandBuffer.pushConstants<uint32_t>(*pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0,
                                              mesh.textureIndex);
        commandBuffer.drawIndexed(mesh.indices.size(), 1, 0, 0, 0);
    }
    commandBuffer.endRendering();

    vk::RenderingAttachmentInfo imGuiAttachmentInfo{};
    imGuiAttachmentInfo.setImageView(swapChainImageViews[imageIndex])
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setLoadOp(vk::AttachmentLoadOp::eLoad)
        .setStoreOp(vk::AttachmentStoreOp::eStore);

    vk::RenderingInfo imGuiRenderingInfo{};
    imGuiRenderingInfo.setRenderArea(renderArea)
        .setLayerCount(1)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&imGuiAttachmentInfo)
        .setPDepthAttachment(nullptr);

    commandBuffer.beginRendering(imGuiRenderingInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);
    commandBuffer.endRendering();

    // After rendering, transition the swapchain image to PRESENT_SRC
    transition_image_layout(swapChainImages[imageIndex], vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits2::eColorAttachmentWrite, {},
                            vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);
    commandBuffer.end();
}

void GNVEngine::transition_image_layout(vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout,
                                        vk::AccessFlags2 src_access_mask, vk::AccessFlags2 dst_access_mask,
                                        vk::PipelineStageFlags2 src_stage_mask, vk::PipelineStageFlags2 dst_stage_mask,
                                        vk::ImageAspectFlags image_aspect_flags)
{
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.setAspectMask(image_aspect_flags)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);
    vk::ImageMemoryBarrier2 barrier{};
    barrier.setSrcStageMask(src_stage_mask)
        .setSrcAccessMask(src_access_mask)
        .setDstStageMask(dst_stage_mask)
        .setDstAccessMask(dst_access_mask)
        .setOldLayout(old_layout)
        .setNewLayout(new_layout)
        .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setImage(image)
        .setSubresourceRange(subresourceRange);
    vk::DependencyInfo dependency_info{};
    dependency_info.setDependencyFlags({}).setImageMemoryBarrierCount(1).setPImageMemoryBarriers(&barrier);
    commandBuffers[frameIndex].pipelineBarrier2(dependency_info);
}

void GNVEngine::createSyncObjects()
{
    assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(device, vk::FenceCreateInfo{ vk::FenceCreateFlagBits::eSignaled });
    }
}

void GNVEngine::drawFrame()
{
    // Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
    //       while renderFinishedSemaphores is indexed by imageIndex
    while (vk::Result::eTimeout == device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX))
        ;
    device.resetFences(*inFlightFences[frameIndex]);

    auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    updateUniformBuffer(frameIndex);

    commandBuffers[frameIndex].reset();
    newImGuiFrame();
    recordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submitInfo{};
    submitInfo.setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&*presentCompleteSemaphores[frameIndex])
        .setPWaitDstStageMask(&waitDestinationStageMask)
        .setCommandBufferCount(1)
        .setPCommandBuffers(&*commandBuffers[frameIndex])
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&*renderFinishedSemaphores[imageIndex]);
    queue.submit(submitInfo, *inFlightFences[frameIndex]);

    try {
        vk::PresentInfoKHR presentInfoKHR{};
        presentInfoKHR.setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&*renderFinishedSemaphores[imageIndex])
            .setSwapchainCount(1)
            .setPSwapchains(&*swapChain)
            .setPImageIndices(&imageIndex);
        result = queue.presentKHR(presentInfoKHR);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    } catch (const vk::SystemError& e) {
        if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR)) {
            recreateSwapChain();
            return;
        } else {
            throw;
        }
    }
    frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

[[nodiscard]] vk::raii::ShaderModule GNVEngine::createShaderModule(const std::vector<char>& code) const
{
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(code.size() * sizeof(char)).setPCode(reinterpret_cast<const uint32_t*>(code.data()));
    vk::raii::ShaderModule shaderModule{ device, createInfo };

    return shaderModule;
}

uint32_t GNVEngine::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
{
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }
    return minImageCount;
}

vk::SurfaceFormatKHR GNVEngine::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    // used to be vk::Format::eB8G8R8A8Srgb, changed to Unorm to match ImGui
    assert(!availableFormats.empty());
    const auto formatIt = std::ranges::find_if(availableFormats, [](const auto& format) {
        return format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    });
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR GNVEngine::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    assert(std::ranges::any_of(availablePresentModes,
                               [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(availablePresentModes,
                               [](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}

vk::Extent2D GNVEngine::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != 0xFFFFFFFF) {
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    return { std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
             std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height) };
}

std::vector<const char*> GNVEngine::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL GNVEngine::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void*)
{
    if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError ||
        severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
    }

    return vk::False;
}

std::vector<char> GNVEngine::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << filename << std::endl;
        throw std::runtime_error("failed to open file ");
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();
    return buffer;
}

void GNVEngine::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                             vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory)
{
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);
    buffer = vk::raii::Buffer(device, bufferInfo);

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));
    bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
}

void GNVEngine::copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(commandPool).setLevel(vk::CommandBufferLevel::ePrimary).setCommandBufferCount(1);
    vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandCopyBuffer.begin(beginInfo);
    commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
    commandCopyBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBufferCount(1).setPCommandBuffers(&*commandCopyBuffer);
    queue.submit(submitInfo, nullptr);

    queue.waitIdle();
}

void GNVEngine::createDescriptorSetLayout()
{
    std::array bindings = { vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1,
                                                           vk::ShaderStageFlagBits::eVertex, nullptr),
                            vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, MAX_TEXTURES,
                                                           vk::ShaderStageFlagBits::eFragment, nullptr) };

    std::array<vk::DescriptorBindingFlags, 2> bindingFlags = {
        vk::DescriptorBindingFlags{},
        vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound |
            vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
    };

    vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT flagsInfo{};
    flagsInfo.setBindingCount(static_cast<uint32_t>(bindingFlags.size())).setPBindingFlags(bindingFlags.data());

    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.setBindingCount(static_cast<uint32_t>(bindings.size()))
        .setPBindings(bindings.data())
        .setPNext(&flagsInfo)
        .setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool);
    descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
}

void GNVEngine::createDepthResources()
{
    findDepthFormat();
    EngineLog::logger->debug("Depth format {}", vk::to_string(depthFormat));

    createImage(swapChainExtent.width, swapChainExtent.height, 1, depthFormat, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage,
                depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
}

vk::Format GNVEngine::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                          vk::FormatFeatureFlags features) const
{
    for (const auto format : candidates) {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void GNVEngine::findDepthFormat()
{
    if (depthFormat == vk::Format::eUndefined) {
        depthFormat =
            findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
                                vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }
}

void GNVEngine::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format,
                            vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
                            vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory)
{
    vk::ImageCreateInfo imageInfo{};
    imageInfo.setImageType(vk::ImageType::e2D)
        .setFormat(format)
        .setExtent({ width, height, 1 })
        .setMipLevels(mipLevels)
        .setArrayLayers(1)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setTiling(tiling)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setInitialLayout(vk::ImageLayout::eUndefined);
    image = vk::raii::Image(device, imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));
    imageMemory = vk::raii::DeviceMemory(device, allocInfo);
    image.bindMemory(*imageMemory, 0);
}

vk::raii::ImageView GNVEngine::createImageView(vk::raii::Image& image, vk::Format format,
                                               vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    vk::ImageSubresourceRange range{};
    range.setAspectMask(vk::ImageAspectFlagBits::eColor).setBaseMipLevel(0).setLevelCount(mipLevels).setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(format).setSubresourceRange(range);
    return vk::raii::ImageView(device, viewInfo);
}

size_t GNVEngine::createTexture(const uint8_t* ktxData, size_t ktxSize)
{
    EngineLog::logger->trace("Creating texture");
    Texture texture{};
    ktxTexture2* kTexture;
    KTX_error_code result =
        ktxTexture2_CreateFromMemory(ktxData, ktxSize, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
    EngineLog::logger->trace("KTX texture loaded from memory");

    if (result != KTX_SUCCESS) {
        throw std::runtime_error("failed to load ktx texture image!");
    }

    if (kTexture->classId != ktxTexture2_c) {
        throw std::runtime_error("not a ktx2 texture!");
    }

    auto* ktx2 = reinterpret_cast<ktxTexture2*>(kTexture);

    if (ktxTexture2_NeedsTranscoding(ktx2)) {
        if (ktxTexture2_TranscodeBasis(ktx2, KTX_TTF_RGBA32, 0) != KTX_SUCCESS)
            throw std::runtime_error("Failed to transcode KTX2 texture to RGBA32");
        texture.imageFormat = vk::Format::eR8G8B8A8Unorm;
    } else {
        texture.imageFormat = static_cast<vk::Format>(ktx2->vkFormat);
    }

    size_t totalSize = kTexture->dataSize;
    uint8_t* ktxTextureData = kTexture->pData;

    texture.mipLevels = kTexture->numLevels;
    if (texture.mipLevels > maxLod) {
        maxLod = texture.mipLevels - 1;
        createTextureSampler();
    }

    texture.width = kTexture->baseWidth;
    texture.height = kTexture->baseHeight;
    EngineLog::logger->trace("KTX texture data loaded");

    vk::raii::Buffer stagingBuffer({});
    vk::raii::DeviceMemory stagingBufferMemory({});
    createBuffer(totalSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
                 stagingBufferMemory);

    void* data = stagingBufferMemory.mapMemory(0, totalSize);
    memcpy(data, ktxTextureData, totalSize);
    stagingBufferMemory.unmapMemory();
    EngineLog::logger->trace("Staging buffer copied");

    std::vector<vk::BufferImageCopy> regions;

    for (uint32_t level = 0; level < texture.mipLevels; level++) {
        ktx_size_t offset = 0;
        ktx_size_t mipSize = ktxTexture2_GetImageOffset(kTexture, level, 0, 0, &offset);

        vk::BufferImageCopy region{};
        region.setBufferOffset(offset).setBufferRowLength(0).setBufferImageHeight(0);

        vk::ImageSubresourceLayers subresourceLayers{};
        subresourceLayers.setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setMipLevel(level)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        region.setImageSubresource(subresourceLayers);
        vk::Extent3D extent{};
        extent.width = kTexture->baseWidth >> level;
        extent.height = kTexture->baseHeight >> level;
        extent.depth = 1;
        region.setImageExtent(extent);

        regions.push_back(region);
    }

    createImage(texture.width, texture.height, texture.mipLevels, texture.imageFormat, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal, texture.image, texture.imageMemory);
    EngineLog::logger->trace("Image created");

    transitionImageLayout(texture.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                          texture.mipLevels);

    auto commandBuffer = beginSingleTimeCommands();
    commandBuffer->copyBufferToImage(stagingBuffer, texture.image, vk::ImageLayout::eTransferDstOptimal, regions);
    endSingleTimeCommands(*commandBuffer);

    transitionImageLayout(texture.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                          texture.mipLevels);
    EngineLog::logger->trace("Transition + copy to image");

    ktxTexture2_Destroy(kTexture);

    texture.imageView =
        createImageView(texture.image, texture.imageFormat, vk::ImageAspectFlagBits::eColor, texture.mipLevels);

    textureManager.push_back(std::move(texture));

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        addTextureToBindless(descriptorSets[i], textureManager.back(),
                             static_cast<uint32_t>(textureManager.size() - 1));
    }
    return textureManager.size() - 1;
}

void GNVEngine::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout,
                                      vk::ImageLayout newLayout, uint32_t mipLevels)
{
    auto commandBuffer = beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    vk::ImageSubresourceRange range{};
    range.setAspectMask(vk::ImageAspectFlagBits::eColor).setBaseMipLevel(0).setLevelCount(mipLevels).setLayerCount(1);
    barrier.setOldLayout(oldLayout).setNewLayout(newLayout).setImage(*image).setSubresourceRange(range);

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    commandBuffer->pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
    endSingleTimeCommands(*commandBuffer);
}

std::unique_ptr<vk::raii::CommandBuffer> GNVEngine::beginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.setCommandPool(*commandPool).setLevel(vk::CommandBufferLevel::ePrimary).setCommandBufferCount(1);
    std::unique_ptr<vk::raii::CommandBuffer> commandBuffer =
        std::make_unique<vk::raii::CommandBuffer>(std::move(vk::raii::CommandBuffers(device, allocInfo).front()));

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer->begin(beginInfo);

    return commandBuffer;
}

void GNVEngine::endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.setCommandBufferCount(1).setPCommandBuffers(&*commandBuffer);
    queue.submit(submitInfo, nullptr);
    queue.waitIdle();
}

void GNVEngine::copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width,
                                  uint32_t height)
{
    std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = beginSingleTimeCommands();
    vk::BufferImageCopy region{};
    region.setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 })
        .setImageOffset({ 0, 0, 0 })
        .setImageExtent({ width, height, 1 });
    commandBuffer->copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, { region });
    endSingleTimeCommands(*commandBuffer);
}

void GNVEngine::createTextureSampler()
{
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setMipLodBias(0.0f)
        .setMinLod(0.0f)
        .setMaxLod(static_cast<float>(maxLod))
        .setAnisotropyEnable(vk::True)
        .setMaxAnisotropy(properties.limits.maxSamplerAnisotropy)
        .setCompareEnable(vk::False)
        .setCompareOp(vk::CompareOp::eAlways);
    textureSampler = vk::raii::Sampler(device, samplerInfo);
}

void GNVEngine::loadModel()
{
    EngineLog::logger->trace("Loading {}", MODEL_PATH);
    std::filesystem::path path{ MODEL_PATH };
    if (!std::filesystem::exists(path)) {
        EngineLog::logger->error("GLB file not found: {}", path.string());
        return;
    }
    static constexpr auto supportedExtensions =
        fastgltf::Extensions::KHR_mesh_quantization | fastgltf::Extensions::KHR_texture_transform |
        fastgltf::Extensions::KHR_materials_variants | fastgltf::Extensions::KHR_texture_basisu |
        fastgltf::Extensions::EXT_meshopt_compression;
    fastgltf::Parser parser{ supportedExtensions };

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                 fastgltf::Options::GenerateMeshIndices;
    auto gltfFile = fastgltf::MappedGltfFile::FromPath(path);
    fastgltf::Asset asset = std::move(parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions).get());
    EngineLog::logger->trace("Images: {}", asset.images.size());
    EngineLog::logger->trace("Textures: {}", asset.textures.size());
    EngineLog::logger->trace("Materials: {}", asset.materials.size());
    EngineLog::logger->trace("Meshes: {}", asset.meshes.size());
    EngineLog::logger->trace("Nodes: {}", asset.nodes.size());

    std::vector<size_t> textureIndices(asset.images.size());
    for (size_t i = 0; i < asset.images.size(); ++i) {
        auto& image = asset.images[i];
        auto& view = std::get<fastgltf::sources::BufferView>(image.data);
        auto& bufferView = asset.bufferViews[view.bufferViewIndex];
        auto& buffer = asset.buffers[bufferView.bufferIndex];
        auto& vector = std::get<fastgltf::sources::Array>(buffer.data);
        auto ktxData = reinterpret_cast<const uint8_t*>(vector.bytes.data() + bufferView.byteOffset);
        size_t ktxSize = bufferView.byteLength;

        textureIndices[i] = createTexture(ktxData, ktxSize);
    }
    EngineLog::logger->trace("Textures loaded");

    for (auto& aMesh : asset.meshes) {
        Mesh mesh{};
        uint32_t offset = 0;

        if (!aMesh.primitives.empty() && aMesh.primitives[0].materialIndex.has_value()) {
            size_t materialIdx = aMesh.primitives[0].materialIndex.value();
            auto& material = asset.materials[materialIdx];
            if (material.pbrData.baseColorTexture.has_value()) {
                size_t imageIdx = material.pbrData.baseColorTexture->textureIndex;
                mesh.textureIndex = textureIndices[imageIdx];
            } else {
                mesh.textureIndex = 0;
            }
        }
        EngineLog::logger->trace("Textures index found {}", mesh.textureIndex);
        EngineLog::logger->trace("Now loading primitives {}", aMesh.primitives.size());

        for (auto& aPrimitive : aMesh.primitives) {
            auto* posAttr = aPrimitive.findAttribute("POSITION");
            size_t primitiveVertexCount = 0;
            size_t baseIndex = mesh.vertices.size();
            if (posAttr) {
                auto& posAccessor = asset.accessors[posAttr->accessorIndex];
                if (posAccessor.type != fastgltf::AccessorType::Vec3) {
                    EngineLog::logger->error("POSITION accessor is not VEC3!");
                }
                primitiveVertexCount = posAccessor.count;
                EngineLog::logger->trace("Attr found POSITION, resizing {}", baseIndex + primitiveVertexCount);
                mesh.vertices.resize(baseIndex + primitiveVertexCount);

                if (!posAccessor.bufferViewIndex.has_value()) {
                    EngineLog::logger->error("Position accessor missing bufferView!");
                }
                EngineLog::logger->trace("Byte offset:{}, Count:{}, Vec size:{}, Byte length:{}",
                                         posAccessor.byteOffset, posAccessor.count, sizeof(fastgltf::math::fvec3),
                                         asset.bufferViews[posAccessor.bufferViewIndex.value()].byteLength);
                glm::vec3 min{};
                glm::vec3 max{};
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
                    asset, posAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
                        glm::vec3 vert{ pos.x(), pos.y(), pos.z() };
                        mesh.vertices[baseIndex + idx].pos = vert;
                        min = glm::min(min, vert);
                        max = glm::max(max, vert);
                    });
                EngineLog::logger->trace("Vertex positions loaded {}, Min:{}, Max:{}", posAccessor.count,
                                         glm::to_string(min), glm::to_string(max));
            }

            auto* texAttr = aPrimitive.findAttribute("TEXCOORD_0");
            if (texAttr != aPrimitive.attributes.end()) {
                auto& texAccessor = asset.accessors[texAttr->accessorIndex];
                if (texAccessor.type != fastgltf::AccessorType::Vec2) {
                    EngineLog::logger->error("TEXTURE accessor is not VEC2: {} AccessorIndex:{}", int(texAccessor.type),
                                             texAttr->accessorIndex);
                }
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
                    asset, texAccessor, [&](fastgltf::math::fvec2 uv, std::size_t idx) {
                        mesh.vertices[baseIndex + idx].texCoord = glm::vec2(uv.x(), uv.y());
                    });
                EngineLog::logger->trace("UVs loaded {}", texAccessor.count);
            } else {
                for (size_t i = 0; i < primitiveVertexCount; ++i)
                    mesh.vertices[baseIndex + i].texCoord = glm::vec2(0.0f);
                EngineLog::logger->trace("UVs loaded empty");
            }

            if (aPrimitive.indicesAccessor.has_value()) {
                auto& indexAccessor = asset.accessors[aPrimitive.indicesAccessor.value()];
                if (indexAccessor.type != fastgltf::AccessorType::Scalar) {
                    EngineLog::logger->error("INDEX accessor is not SCALAR!");
                }
                if (!indexAccessor.bufferViewIndex.has_value())
                    throw std::runtime_error("Index accessor missing buffer view");
                size_t oldIndexCount = mesh.indices.size();
                mesh.indices.resize(oldIndexCount + indexAccessor.count);

                if (indexAccessor.componentType == fastgltf::ComponentType::UnsignedByte ||
                    indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort) {
                    std::vector<uint16_t> tempIndices(indexAccessor.count);
                    fastgltf::copyFromAccessor<uint16_t>(asset, indexAccessor, tempIndices.data());
                    for (size_t i = 0; i < tempIndices.size(); ++i) {
                        mesh.indices[oldIndexCount + i] = baseIndex + static_cast<uint32_t>(tempIndices[i]);
                    }
                } else if (indexAccessor.componentType == fastgltf::ComponentType::UnsignedInt) {
                    std::vector<uint32_t> tempIndices(indexAccessor.count);
                    fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, tempIndices.data());
                    for (size_t i = 0; i < tempIndices.size(); ++i) {
                        mesh.indices[oldIndexCount + i] = baseIndex + tempIndices[i];
                    }
                } else {
                    throw std::runtime_error("Unsupported index type in glTF");
                }
                EngineLog::logger->trace("Indices loaded {}", indexAccessor.count);
            }
            offset += primitiveVertexCount;
        }
        createVertexBuffer(mesh);
        createIndexBuffer(mesh);
        meshManager.push_back(std::move(mesh));
    }
}

void GNVEngine::createUniformBuffers()
{
    uniformBuffers.clear();
    uniformBuffersMemory.clear();
    uniformBuffersMapped.clear();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
        vk::raii::Buffer buffer({});
        vk::raii::DeviceMemory bufferMem({});
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer,
                     bufferMem);
        uniformBuffers.emplace_back(std::move(buffer));
        uniformBuffersMemory.emplace_back(std::move(bufferMem));
        uniformBuffersMapped.emplace_back(uniformBuffersMemory[i].mapMemory(0, bufferSize));
    }
}

void GNVEngine::createDescriptorSets()
{
    uint32_t textureCount = static_cast<uint32_t>(textureManager.size());

    std::vector<uint32_t> variableCounts(MAX_FRAMES_IN_FLIGHT, MAX_TEXTURES);
    vk::DescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
    variableCountInfo.setDescriptorSetCount(static_cast<uint32_t>(variableCounts.size()))
        .setPDescriptorCounts(variableCounts.data());

    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.setDescriptorPool(*descriptorPool)
        .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
        .setPSetLayouts(layouts.data())
        .setPNext(&variableCountInfo);

    descriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::vector<vk::WriteDescriptorSet> writes{};

        // UBO
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.setBuffer(*uniformBuffers[i]).setOffset(0).setRange(sizeof(UniformBufferObject));

        vk::WriteDescriptorSet uboWrite{};
        uboWrite.setDstSet(*descriptorSets[i])
            .setDstBinding(0)
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setPBufferInfo(&bufferInfo);
        writes.push_back(uboWrite);

        // Texture array
        std::vector<vk::DescriptorImageInfo> imageInfos;
        for (auto& texture : textureManager) {
            vk::DescriptorImageInfo info{};
            info.setSampler(*textureSampler)
                .setImageView(texture.imageView)
                .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            imageInfos.push_back(info);
        }

        vk::WriteDescriptorSet textureWrite{};
        if (!imageInfos.empty()) {
            textureWrite.setDstSet(*descriptorSets[i])
                .setDstBinding(1)
                .setDescriptorCount(static_cast<uint32_t>(imageInfos.size()))
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setPImageInfo(imageInfos.data());
            writes.push_back(textureWrite);
        }

        // std::array<vk::WriteDescriptorSet, 2> writes = { uboWrite, textureWrite };
        device.updateDescriptorSets(writes, {});
    }
}

uint32_t GNVEngine::addTextureToBindless(vk::raii::DescriptorSet& descriptorSet, Texture& tex, uint32_t slot)
{
    EngineLog::logger->trace("Adding texture to slot {}", slot);
    vk::DescriptorImageInfo info{};
    info.setSampler(*textureSampler)
        .setImageView(tex.imageView)
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    vk::WriteDescriptorSet write{};
    write.setDstSet(descriptorSet)
        .setDstBinding(1)
        .setDstArrayElement(slot)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setPImageInfo(&info);
    device.updateDescriptorSets(write, {});
    return slot;
}

void GNVEngine::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    // glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
    // glm::mat4 continuousRotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0, 0, 1));
    // ubo.model = continuousRotation * initialRotation;
    ubo.model = glm::identity<glm::mat4>();

    ubo.view = glm::lookAt(camera.position, camera.target, camera.up);

    ubo.proj = glm::perspective(glm::radians(camera.fov),
                                static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height),
                                0.1f, 100.0f);

    // ubo.model = glm::mat4(1.0f);
    // ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    // ubo.proj = glm::perspective(glm::radians(45.0f),
    //                             static_cast<float>(swapChainExtent.width) /
    //                             static_cast<float>(swapChainExtent.height), 0.1f, 100.0f);

    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void GNVEngine::newImGuiFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin(ENGINE_NAME.c_str());

    if (ImGui::CollapsingHeader("Log")) {
        if (EngineLog::imgui_sink != nullptr) {
            for (auto& entry : EngineLog::imgui_sink->buffer)
                ImGui::TextColored(entry.color, "%s", entry.msg.c_str());
        }
    }

    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::SliderFloat3("Position", &camera.position.x, -10.0f, 10.0f);
        ImGui::SliderFloat3("Target", &camera.target.x, -10.0f, 10.0f);
        ImGui::SliderFloat3("Up", &camera.up.x, -1.0f, 1.0f);
        ImGui::SliderFloat("FOV", &camera.fov, 1.0f, 120.0f);
        if (ImGui::CollapsingHeader("Camera Matrix")) {
            std::string text = "Model:\n" + glm::to_string(ubo.model) + "\n\n" + "View:\n" + glm::to_string(ubo.view) +
                               "\n\n" + "Proj:\n" + glm::to_string(ubo.proj);
            ImGui::TextUnformatted(text.c_str());
        }
    }

    if (ImGui::CollapsingHeader("Mesh Data")) {
        for (size_t m = 0; m < meshManager.size(); ++m) {
            auto& mesh = meshManager[m];
            if (ImGui::TreeNode((void*)(intptr_t)m, "Mesh %zu", m)) {

                ImGui::Text("Texture index: %zu", mesh.textureIndex);

                // Vertices
                if (ImGui::TreeNode("Vertices")) {
                    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
                        auto& v = mesh.vertices[i];
                        ImGui::Text("[%zu] pos=(%.3f, %.3f, %.3f) uv=(%.3f, %.3f)", i, v.pos.x, v.pos.y, v.pos.z,
                                    v.texCoord.x, v.texCoord.y);
                    }
                    ImGui::TreePop();
                }

                // Indices
                if (ImGui::TreeNode("Indices")) {
                    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                        if (i + 2 < mesh.indices.size()) {
                            ImGui::Text("[%zu] %u, %u, %u", i / 3, mesh.indices[i], mesh.indices[i + 1],
                                        mesh.indices[i + 2]);
                        }
                    }
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
    ImGui::Render();
}
