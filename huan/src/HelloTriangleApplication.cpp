//
// Created by 86156 on 4/4/2025.
//

#include "huan/HelloTriangleApplication.hpp"

#include <set>
#include <GLFW/glfw3.h>
#include "huan/settings.hpp"
#include "huan/backend/shader.hpp"
#include "huan/log/Log.hpp"
#include "huan/utils/file_load.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        HUAN_CORE_ERROR("validation layer: {}", pCallbackData->pMessage)
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        HUAN_CORE_WARN("validation layer: {}", pCallbackData->pMessage)
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        HUAN_CORE_INFO("validation layer: {}", pCallbackData->pMessage)
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        HUAN_CORE_TRACE("validation layer: {}", pCallbackData->pMessage)
    else
        HUAN_CORE_WARN("validation layer: {}", pCallbackData->pMessage)

    return VK_FALSE;
}

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator,
                                        VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = huan::HelloTriangleApplication::getInstance();
    app->m_framebufferResized = true;
}

namespace huan
{
    void HelloTriangleApplication::initLogSystem()
    {
        Log::init();
    }

    void HelloTriangleApplication::init()
    {
        if (isInitialized())
            return;

        initLogSystem();
        initWindow();
        initVulkan();

        initialized = true;
    }

    void HelloTriangleApplication::run()
    {
        mainLoop();
    }

    HelloTriangleApplication::HelloTriangleApplication()
    {
        // nothing
    }

    void HelloTriangleApplication::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(globalAppSettings.width, globalAppSettings.height, globalAppSettings.title,
                                  nullptr,
                                  nullptr);

        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void HelloTriangleApplication::createCommandPool()
    {
        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                             .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());
        m_commandPool = device.createCommandPool(commandPoolCreateInfo);
        if (!m_commandPool)
            HUAN_CORE_ERROR("Failed to create command pool");

        HUAN_CORE_INFO("Created command pool");
    }

    void HelloTriangleApplication::createCommandBuffer()
    {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.setCommandPool(m_commandPool)
                                 .setLevel(vk::CommandBufferLevel::ePrimary)
                                 .setCommandBufferCount(1);
        for (size_t i = 0; i < globalAppSettings.maxFramesInFlight; ++i)
        {
            m_frameDatas[i].m_commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo)[0];
        }

        HUAN_CORE_INFO("Created command buffer");
    }

    void HelloTriangleApplication::createSynchronization()
    {
        vk::SemaphoreCreateInfo semaphoreCreateInfo;
        vk::FenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

        for (size_t i = 0; i < globalAppSettings.maxFramesInFlight; ++i)
        {
            m_frameDatas[i].m_imageAvailableSemaphore = device.createSemaphore(semaphoreCreateInfo);
            m_frameDatas[i].m_renderFinishedSemaphore = device.createSemaphore(semaphoreCreateInfo);
            m_frameDatas[i].m_fence = device.createFence(fenceCreateInfo);
        }

        HUAN_CORE_INFO("Created synchronization");
    }

    void HelloTriangleApplication::createFrameData()
    {
        m_frameDatas.resize(globalAppSettings.maxFramesInFlight);
        createCommandBuffer();
        createSynchronization();
    }

    std::vector<const char*> HelloTriangleApplication::getRequiredInstanceExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (globalAppSettings.isVulkanValidationEnabled)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return std::move(extensions);
    }

    std::vector<const char*> HelloTriangleApplication::getRequiredDeviceExtensions()
    {
        return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    }

    void HelloTriangleApplication::createInstance()
    {
        vk::InstanceCreateInfo vkInstanceCreateInfo;
        vk::ApplicationInfo appInfo;

        const auto availableExtensions = vk::enumerateInstanceExtensionProperties();
        const auto availableLayers = vk::enumerateInstanceLayerProperties();

        std::ostringstream infoString;
        infoString << "\nAvailable Extensions: \n";
        for (const auto& extension : availableExtensions)
        {
            infoString << "\t" << extension.extensionName << "\n";
        }
        infoString << "\nAvailable Layers: \n";
        for (const auto& layer : availableLayers)
        {
            infoString << "\t" << layer.layerName << "\n";
        }
        HUAN_CORE_INFO(infoString.str());
        infoString.str("");

        // Prepare the required extensions and layers
        std::vector<const char*> requiredInstanceExtensions = getRequiredInstanceExtensions();
        std::vector<const char*> requiredLayers;
        infoString << "\nRequired Instance Extensions: \n";
        for (const auto& requiredExtension : requiredInstanceExtensions)
            infoString << "\t" << requiredExtension << "\n";

        infoString << "\nRequired Layers: \n";
        if (globalAppSettings.isVulkanValidationEnabled)
        {
            requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
            infoString << "\t" << requiredLayers.back() << "\n";
        }

        // Print all required extensions and layers
        HUAN_CORE_INFO(infoString.str());

        // Check if the required extensions and layers are available
        for (const auto& requiredExtension : requiredInstanceExtensions)
        {
            bool found = false;
            for (const auto& extension : availableExtensions)
            {
                if (strcmp(requiredExtension, extension.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                HUAN_CORE_ERROR("Required extension {} is not available", requiredExtension);
            }
        }
        for (const auto& requiredLayer : requiredLayers)
        {
            bool found = false;
            for (const auto& layer : availableLayers)
            {
                if (strcmp(requiredLayer, layer.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                HUAN_CORE_ERROR("Required layer {} is not available", requiredLayer);
            }
        }


        appInfo.setApiVersion(vk::ApiVersion13)
               .setPApplicationName(globalAppSettings.title)
               .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
               .setPEngineName("HuanRenderer")
               .setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

        vkInstanceCreateInfo.setPApplicationInfo(&appInfo)
                            .setEnabledExtensionCount(requiredInstanceExtensions.size())
                            .setPpEnabledExtensionNames(requiredInstanceExtensions.data())
                            .setEnabledLayerCount(requiredLayers.size())
                            .setPpEnabledLayerNames(requiredLayers.data());

        if (globalAppSettings.isVulkanValidationEnabled)
        {
            debugCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
                           .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                               vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                               vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
                           .setPfnUserCallback(debugCallback);
            vkInstanceCreateInfo.setPNext(&debugCreateInfo);
        }

        vkInstance = vk::createInstance(vkInstanceCreateInfo);
        if (!vkInstance)
        {
            HUAN_CORE_BREAK("Failed to create Vulkan instance")
        }
    }

    void HelloTriangleApplication::createDebugMessenger()
    {
        if (!globalAppSettings.isVulkanValidationEnabled)
            return;
        debugMessenger = vkInstance.createDebugUtilsMessengerEXT(debugCreateInfo);
    }

    void HelloTriangleApplication::pickPhysicalDevice()
    {
        const auto devices = vkInstance.enumeratePhysicalDevices();
        for (const auto& device : devices)
        {
            if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                physicalDevice = device;
                break;
            }
        }
        if (!physicalDevice)
            HUAN_CORE_BREAK("Failed to find a discrete GPU")

        HUAN_CORE_INFO("Picked physical device: {}", physicalDevice.getProperties().deviceName.data())
        std::vector<vk::ExtensionProperties> deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        std::ostringstream infoString;
        infoString << "\nAvailable Device Extensions: \n";
        for (const auto& extension : deviceExtensions)
        {
            infoString << "\t" << extension.extensionName << "\n";
        }
        HUAN_CORE_INFO(infoString.str())
        infoString.str("");
        auto requiredDeviceExtensions = getRequiredDeviceExtensions();
        infoString << "\nRequired Device Extensions: \n";
        for (const auto& deviceExtension : deviceExtensions)
            infoString << "\t" << deviceExtension.extensionName << "\n";
        HUAN_CORE_INFO(infoString.str());

        for (const auto& requiredDeviceExtension : requiredDeviceExtensions)
        {
            bool found = false;
            for (const auto& extension : deviceExtensions)
            {
                if (strcmp(requiredDeviceExtension, extension.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                HUAN_CORE_BREAK("Required device extension {} is not available", requiredDeviceExtension);
            }
        }
        HUAN_CORE_INFO("All required device extensions are available! ")
    }

    void HelloTriangleApplication::createDevice()
    {
        auto requiredDeviceExtensions = getRequiredDeviceExtensions();
        vk::DeviceCreateInfo deviceCreateInfo;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        float priorities = 1.0f;
        std::set<uint32_t> uniqueQueueFamilies = {
            queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()
        };
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            vk::DeviceQueueCreateInfo queueCreateInfo;

            queueCreateInfo.setPQueuePriorities(&priorities)
                           .setQueueCount(1)
                           .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());
            queueCreateInfos.push_back(queueCreateInfo);
        }

        deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
                        .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
                        .setEnabledExtensionCount(static_cast<uint32_t>(requiredDeviceExtensions.size()))
                        .setPpEnabledExtensionNames(requiredDeviceExtensions.data());

        device = physicalDevice.createDevice(deviceCreateInfo);
    }

    void HelloTriangleApplication::getQueues()
    {
        device.getQueue(queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
        device.getQueue(queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
    }

    void HelloTriangleApplication::createSurface()
    {
        VkSurfaceKHR tempSurface;
        const auto res = glfwCreateWindowSurface(vkInstance, window, nullptr, &tempSurface);
        if (res != VK_SUCCESS)
            HUAN_CORE_BREAK("Failed to create window surface")

        surface = vk::SurfaceKHR(tempSurface);
    }

    void HelloTriangleApplication::createSwapchain()
    {
        swapchain = Swapchain::create(globalAppSettings.width, globalAppSettings.height);
    }

    void HelloTriangleApplication::createGraphicsPipeline()
    {
        HUAN_CORE_INFO("Creating graphics pipeline...")
        // Shaders
        auto vertexShader = utils::loadFile("../../../../assets/HelloTriangle/simple_triangle.vert.spv");
        auto fragShader = utils::loadFile("../../../../assets/HelloTriangle/simple_triangle.frag.spv");

        auto vertexShaderModule = Shader::createShaderModule(vertexShader);
        auto fragShaderModule = Shader::createShaderModule(fragShader);

        vk::PipelineShaderStageCreateInfo vertexShaderStageInfo;
        vertexShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex)
                             .setModule(vertexShaderModule)
                             .setPName("main");
        vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
        fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment)
                           .setModule(fragShaderModule)
                           .setPName("main");
        vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragShaderStageInfo};

        // Dynamic states
        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState;
        dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
                    .setPDynamicStates(dynamicStates.data());
        // Input state
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.setVertexBindingDescriptionCount(0)
                       .setVertexAttributeDescriptionCount(0);
        // Input assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
                     .setPrimitiveRestartEnable(false);
        // Viewport and scissor
        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.setViewportCount(1)
                     .setScissorCount(1);
        viewportState.setPViewports(&swapchain->getViewport())
                     .setPScissors(&swapchain->getScissor());

        // Rasterizer
        vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
        rasterizerInfo.setDepthClampEnable(false)
                      .setRasterizerDiscardEnable(false)
                      .setPolygonMode(vk::PolygonMode::eFill)
                      .setCullMode(vk::CullModeFlagBits::eBack)
                      .setFrontFace(vk::FrontFace::eClockwise)
                      .setDepthBiasEnable(false)
                      .setLineWidth(1.0f);

        // Multisampling
        vk::PipelineMultisampleStateCreateInfo multisamplingInfo;
        multisamplingInfo.setSampleShadingEnable(false)
                         .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                         .setMinSampleShading(1.0f)
                         .setPSampleMask(nullptr)
                         .setAlphaToCoverageEnable(false)
                         .setAlphaToOneEnable(false);

        // Depth and stencil testing
        // TODO: for now, nullptr

        // Color Blend for every attachment 
        /*
             if (blendEnable) {
                finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
                finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
            } else {
                finalColor = newColor;
            }
    
            finalColor = finalColor & colorWriteMask;
         */
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.setColorWriteMask(
                                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                            .setBlendEnable(false)
                            .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                            .setDstColorBlendFactor(vk::BlendFactor::eZero)
                            .setColorBlendOp(vk::BlendOp::eAdd)
                            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                            .setAlphaBlendOp(vk::BlendOp::eAdd);

        // Global Color blend state
        vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
        colorBlendInfo.setLogicOpEnable(false)
                      .setLogicOp(vk::LogicOp::eCopy)
                      .setAttachmentCount(1)
                      .setPAttachments(&colorBlendAttachment)
                      .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayoutCount(0)
                          .setPushConstantRangeCount(0)
                          .setPushConstantRanges(nullptr)
                          .setPushConstantRangeCount(0);
        m_pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
        if (!m_pipelineLayout)
            HUAN_CORE_BREAK("Failed to create pipeline layout")

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.setStageCount(2)
                    .setPStages(shaderStages)
                    .setPVertexInputState(&vertexInputInfo)
                    .setPInputAssemblyState(&inputAssembly)
                    .setPViewportState(&viewportState)
                    .setPRasterizationState(&rasterizerInfo)
                    .setPMultisampleState(&multisamplingInfo)
                    .setPColorBlendState(&colorBlendInfo)
                    .setPDynamicState(&dynamicState)
                    .setLayout(m_pipelineLayout)
                    .setRenderPass(m_renderPass)
                    .setSubpass(0)
                    .setBasePipelineHandle(nullptr)
                    .setBasePipelineIndex(-1);

        auto pipelineRes = device.createGraphicsPipeline(nullptr, pipelineInfo);
        if (pipelineRes.result != vk::Result::eSuccess)
            HUAN_CORE_BREAK("Failed to create graphics pipeline")

        // Many vulkan objects are just handle, so don't worry about the efficiency.
        m_graphicsPipeline = pipelineRes.value;
        HUAN_CORE_INFO("Graphics pipeline created!")

        device.destroyShaderModule(vertexShaderModule);
        device.destroyShaderModule(fragShaderModule);
    }

    /**
     * Create RenderPass, telling the render pipeline how many color and depth buffers to use, how many samples to use for them and how their contents should be handled through the pipeline.
     */
    void HelloTriangleApplication::createRenderPass()
    {
        vk::AttachmentDescription colorAttachment;
        colorAttachment.setFormat(swapchain->getImageFormat())
                       .setSamples(vk::SampleCountFlagBits::e1)
                       .setLoadOp(vk::AttachmentLoadOp::eClear) // Before rendering
                       .setStoreOp(vk::AttachmentStoreOp::eStore)
                       // After rendering, we want to keep the contents for display on the screen
                       .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                       .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                       .setInitialLayout(vk::ImageLayout::eUndefined)
                       // the format of framebuffer or texture. Initial layout specifies the layout the image will have before the render pass begin. 
                       .setFinalLayout(vk::ImageLayout::ePresentSrcKHR); // Auto transition to when the RenderPass ends.
        // NOTE: we don't care about the frame buffer's layout before the render pass begins, because we will clear it when load. But after the render pass ends, we want to transition the image to the layout that is optimal for presentation to the screen.

        vk::AttachmentReference colorAttachmentRef;
        // set the attachment index in attachment descriptions
        /* The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive */
        colorAttachmentRef.setAttachment(0)
                          .setLayout(vk::ImageLayout::eColorAttachmentOptimal);
        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
               .setColorAttachments(colorAttachmentRef)
               .setColorAttachmentCount(1);

        vk::SubpassDependency subpassDependency;
        // this dependency is used to make sure that the color attachment is ready before the beginning of the subpass.
        // The VK_SUBPASS_EXTERNAL constant is used to indicate the external subpasses, which are the subpasses that are not part of the render pass.
        // And if the dst is 0, it means before the subpass. So if src is 0, it means after the subpasses.
        subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                         .setDstSubpass(0)
                         .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                         .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                         .setSrcAccessMask(vk::AccessFlagBits::eNone)
                         .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo renderPassInfo;
        renderPassInfo.setAttachmentCount(1)
                      .setPAttachments(&colorAttachment)
                      .setSubpassCount(1)
                      .setPSubpasses(&subpass)
                      .setDependencies(subpassDependency);
        m_renderPass = device.createRenderPass(renderPassInfo);
        if (!m_renderPass)
            HUAN_CORE_BREAK("Failed to create render pass.")
    }

    void HelloTriangleApplication::createFramebuffers()
    {
        m_swapchainFramebuffers.resize(swapchain->m_imageViews.size());
        // Why I should use swapchain->m_imageViews[i], instead of m_image?
        // Because the imageView essentially is a wrapper of the correspond image in the swapchain. (or a super set).
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.setRenderPass(m_renderPass)
                       .setAttachmentCount(1)
                       .setWidth(swapchain->m_info.extent.width)
                       .setHeight(swapchain->m_info.extent.height)
                       .setLayers(1); // The number of layers of the imageView.
        for (size_t i = 0; i < swapchain->m_imageViews.size(); i++)
        {
            framebufferInfo.setPAttachments(&swapchain->m_imageViews[i]);

            m_swapchainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
            if (!m_swapchainFramebuffers[i])
                HUAN_CORE_BREAK("Failed to create framebuffer")
        }
    }

    void HelloTriangleApplication::drawFrame()
    {
        auto& curInFlightFence = m_frameDatas[m_currentFrame].m_fence;
        auto& curImageAvailableSemaphore = m_frameDatas[m_currentFrame].m_imageAvailableSemaphore;
        auto& curRenderFinishedSemaphore = m_frameDatas[m_currentFrame].m_renderFinishedSemaphore;
        auto& curCommandBuffer = m_frameDatas[m_currentFrame].m_commandBuffer;

        auto resWaitFences = device.waitForFences(curInFlightFence, true, UINT64_MAX);

        if (resWaitFences != vk::Result::eSuccess)
            HUAN_CORE_ERROR("Failed to wait for fences")

        uint32_t imageIndex;
        auto resAcqNextImage = swapchain->acquireNextImage(UINT64_MAX,
                                                           curImageAvailableSemaphore, VK_NULL_HANDLE, imageIndex);
        if (resAcqNextImage == vk::Result::eErrorOutOfDateKHR || m_framebufferResized)
        {
            HUAN_CORE_WARN("Swapchain is out of date")
            device.destroySemaphore(curImageAvailableSemaphore);
            curImageAvailableSemaphore = device.createSemaphore({});
            recreateSwapchain();
            return;
        }
        if (resAcqNextImage != vk::Result::eSuccess && resAcqNextImage != vk::Result::eSuboptimalKHR)
            HUAN_CORE_BREAK("Failed to acquire next image")

        // Reset
        device.resetFences(curInFlightFence);
        curCommandBuffer.reset();

        recordCommandBuffer(curCommandBuffer, imageIndex);

        vk::SubmitInfo submitInfo;
        vk::Semaphore waitSemaphores[] = {curImageAvailableSemaphore};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        submitInfo.setWaitSemaphores(waitSemaphores);
        submitInfo.setWaitDstStageMask(waitStages);
        submitInfo.setCommandBuffers(curCommandBuffer);
        vk::Semaphore signalSemaphores[] = {curRenderFinishedSemaphore};
        submitInfo.setSignalSemaphores(signalSemaphores);

        if (graphicsQueue.submit(1, &submitInfo, curInFlightFence) != vk::Result::eSuccess)
            HUAN_CORE_ERROR("Failed to submit draw command buffer")

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(signalSemaphores);
        presentInfo.setSwapchains(swapchain->m_swapchain)
                   .setImageIndices(imageIndex);

        auto resPresent = presentQueue.presentKHR(presentInfo);
        if (resPresent == vk::Result::eErrorOutOfDateKHR || resPresent == vk::Result::eSuboptimalKHR ||
            m_framebufferResized)
        {
            HUAN_CORE_WARN("Swapchain is out of date")
            recreateSwapchain();
        }
        else if (resPresent != vk::Result::eSuccess)
            HUAN_CORE_BREAK("Failed to present image")

        m_currentFrame = (m_currentFrame + 1) % m_frameDatas.size();
    }

    void HelloTriangleApplication::queryQueueFamilyIndices()
    {
        const auto properties = physicalDevice.getQueueFamilyProperties();
        for (auto i = 0; i < properties.size(); i++)
        {
            if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                queueFamilyIndices.graphicsFamily = i;
            }
            if (physicalDevice.getSurfaceSupportKHR(i, surface))
            {
                queueFamilyIndices.presentFamily = i;
            }
            if (queueFamilyIndices.isComplete())
                break;
        }
        if (!queueFamilyIndices.isComplete())
            HUAN_CORE_BREAK("Failed to find a queue family that supports both graphics and presentation")
    }


    void HelloTriangleApplication::initVulkan()
    {
        createInstance();
        createDebugMessenger();

        createSurface();

        pickPhysicalDevice();
        queryQueueFamilyIndices();
        createDevice();
        getQueues();

        createSwapchain();

        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();

        createCommandPool();

        createFrameData();

        HUAN_CORE_TRACE(R"(

    __  __                     ____                 __                   
   / / / /_  ______ _____     / __ \___  ____  ____/ /__  ________  _____
  / /_/ / / / / __ `/ __ \   / /_/ / _ \/ __ \/ __  / _ \/ ___/ _ \/ ___/
 / __  / /_/ / /_/ / / / /  / _, _/  __/ / / / /_/ /  __/ /  /  __/ /    
/_/ /_/\__,_/\__,_/_/ /_/  /_/ |_|\___/_/ /_/\__,_/\___/_/   \___/_/     
                                                                               
                                                                               )"
        );
    }

    void HelloTriangleApplication::mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            drawFrame();
            glfwPollEvents();
        }
    }

    void HelloTriangleApplication::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
    {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setPInheritanceInfo(nullptr);

        if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess)
            HUAN_CORE_BREAK("Failed to begin recording command buffer.")

        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.setRenderPass(m_renderPass)
                      .setFramebuffer(m_swapchainFramebuffers[imageIndex])
                      .setRenderArea(vk::Rect2D{{}, swapchain->m_info.extent});
        vk::ClearValue clearColor = {
            {129.0 / 255.0, 216.0 / 255.0, 207.0 / 255.0, 1.0f}
        };
        renderPassInfo.setClearValues(clearColor);
        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);
        vk::Viewport viewport{
            0, 0, (float)swapchain->m_info.extent.width, (float)swapchain->m_info.extent.height, 0, 1
        };
        commandBuffer.setViewport(0, 1, &viewport);
        vk::Rect2D scissor{{0, 0}, swapchain->m_info.extent};
        commandBuffer.setScissor(0, 1, &scissor);

        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRenderPass();

        commandBuffer.end();
    }

    void HelloTriangleApplication::recreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
        device.waitIdle();
        // TODO: Recreate renderpass
        // It's necessary when you move the renderpass from a standard range to a high dynamic range monitor.

        // Cleanup swapchain
        for (auto& swapchainFramebuffer : m_swapchainFramebuffers)
        {
            device.destroyFramebuffer(swapchainFramebuffer);
        }
        swapchain.reset();

        // Create
        createSwapchain();
        createFramebuffers();
        m_framebufferResized = false;
    }

    void HelloTriangleApplication::cleanup()
    {
        HUAN_CORE_INFO("Cleaning up...\n\n")
        device.waitIdle();

        for (auto& frameData : m_frameDatas)
        {
            device.destroyFence(frameData.m_fence);
            device.destroySemaphore(frameData.m_renderFinishedSemaphore);
            device.destroySemaphore(frameData.m_imageAvailableSemaphore);
        }
        HUAN_CORE_INFO("Synchronizations destroyed.")

        device.destroyCommandPool(m_commandPool);
        HUAN_CORE_INFO("CommandPool destroyed.")
        for (auto& framebuffer : m_swapchainFramebuffers)
        {
            device.destroyFramebuffer(framebuffer);
        }
        HUAN_CORE_INFO("Framebuffers destroyed.")
        device.destroyPipeline(m_graphicsPipeline);
        HUAN_CORE_INFO("Graphics pipeline destroyed.")
        device.destroyPipelineLayout(m_pipelineLayout);
        HUAN_CORE_INFO("Pipeline layout destroyed.")
        device.destroyRenderPass(m_renderPass);
        HUAN_CORE_INFO("Render pass destroyed.")
        swapchain.reset();
        HUAN_CORE_INFO("Swapchain destroyed.")
        vkInstance.destroySurfaceKHR(surface);
        HUAN_CORE_INFO("Surface destroyed.")
        device.destroy();
        HUAN_CORE_INFO("Device destroyed.")

        if (debugMessenger)
        {
            vkInstance.destroyDebugUtilsMessengerEXT(debugMessenger);
            HUAN_CORE_INFO("Debug messenger destroyed.")
        }

        vkInstance.destroy();
        HUAN_CORE_INFO("Instance destroyed.")

        glfwDestroyWindow(window);
        HUAN_CORE_INFO("Window destroyed.")

        glfwTerminate();
    }

    HelloTriangleApplication::~HelloTriangleApplication()
    {
    }
}
