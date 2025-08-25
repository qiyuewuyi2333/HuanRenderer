//
// Created by 86156 on 4/4/2025.
//

#include "huan/VulkanContext.hpp"
#include <set>
#include <chrono>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "huan/settings.hpp"
#include "huan/backend/shader.hpp"
#include "huan/backend/resource/vulkan_buffer.hpp"
#include "huan/log/Log.hpp"
#include "huan/utils/file_load.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "../include/huan/backend/resource/resource_system.hpp"
#include "huan/backend/resource/vulkan_image_view.hpp"
#include "huan/utils/stb_image.h"
#include "huan/utils/tiny_obj_loader.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

// NOTE: We don't use volk.
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
    auto app = huan::VulkanContext::getInstance();
    app->m_framebufferResized = true;
}

namespace huan
{
void VulkanContext::initLogSystem()
{
    Log::init();
}

void VulkanContext::init()
{
    if (isInitialized())
        return;

    initLogSystem();
    initWindow();
    initVulkan();

    initialized = true;
}

void VulkanContext::run()
{
    mainLoop();
}

VulkanContext::VulkanContext()
{
    // nothing
}

void VulkanContext::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window =
        glfwCreateWindow(globalAppSettings.width, globalAppSettings.height, globalAppSettings.title, nullptr, nullptr);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void VulkanContext::createCommandPool()
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                         .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());
    m_commandPool = device.createCommandPool(commandPoolCreateInfo);
    if (!m_commandPool)
        HUAN_CORE_ERROR("Failed to create graphics command pool");
    commandPoolCreateInfo.setQueueFamilyIndex(queueFamilyIndices.transferFamily.value());
    m_transferCommandPool = device.createCommandPool(commandPoolCreateInfo);
    if (!m_transferCommandPool)
        HUAN_CORE_ERROR("Failed to create transfer command pool");

    HUAN_CORE_INFO("Created command pool");
}

void VulkanContext::createCommandBuffer()
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

void VulkanContext::createSynchronization()
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

void VulkanContext::createFrameData()
{
    m_frameDatas.resize(globalAppSettings.maxFramesInFlight);
    createCommandBuffer();
    createSynchronization();
    createUniformBuffers();
    createDescriptorSets();
}

/**
 * NOTE: 我们使用StagingBuffer作为传输到VertexBuffer的中介缓冲，从而使 后者 不需要被主机可见，从而使用更加高效的内存区域
 */
void VulkanContext::createVertexBufferAndMemory()
{
    const vk::DeviceSize bufferSize = sizeof(Vertex) * m_vertices.size();

    m_vertexBuffer = runtime::ResourceSystem::getInstance()->createDeviceLocalBuffer(
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        bufferSize, (void*)(m_vertices.data()));
    HUAN_CORE_INFO("VertexBuffer created.")
}

void VulkanContext::createIndexBufferAndMemory()
{
    vk::DeviceSize bufferSize = sizeof(uint32_t) * m_indices.size();

    m_indexBuffer = runtime::ResourceSystem::getInstance()->createDeviceLocalBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        bufferSize, (void*)(m_indices.data()));
    HUAN_CORE_INFO("IndexBuffer created.")
}

/**
 * Create uniform buffer for per frames.
 * Because vulkan can render in parallel, so we need to do that.
 */
void VulkanContext::createUniformBuffers()
{
    const vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < globalAppSettings.maxFramesInFlight; ++i)
    {
        m_frameDatas[i].m_uniformBuffer = runtime::ResourceSystem::getInstance()->createStagingBuffer(
            vk::BufferUsageFlagBits::eUniformBuffer,
            bufferSize, nullptr);
    }

    HUAN_CORE_INFO("UniformBuffers created. ")
}

std::vector<const char*> VulkanContext::getRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (globalAppSettings.isVulkanValidationEnabled)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return std::move(extensions);
}

std::vector<const char*> VulkanContext::getRequiredDeviceExtensions()
{
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

void VulkanContext::createInstance()
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
    const std::vector<const char*> requiredInstanceExtensions = getRequiredInstanceExtensions();
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
        debugCreateInfo
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
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

void VulkanContext::createDebugMessenger()
{
    if (!globalAppSettings.isVulkanValidationEnabled)
        return;
    debugMessenger = vkInstance.createDebugUtilsMessengerEXT(debugCreateInfo);
}

void VulkanContext::pickPhysicalDevice()
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
    for (const auto& deviceExtension : requiredDeviceExtensions)
        infoString << "\t" << deviceExtension << "\n";
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

void VulkanContext::createDevice()
{
    auto requiredDeviceExtensions = getRequiredDeviceExtensions();
    vk::DeviceCreateInfo deviceCreateInfo;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float priorities = 1.0f;
    std::set uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value(),
                                    queueFamilyIndices.transferFamily.value()};

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;

        queueCreateInfo.setQueuePriorities(priorities).setQueueFamilyIndex(queueFamily);
        queueCreateInfos.push_back(queueCreateInfo);
    }
    vk::PhysicalDeviceFeatures features;
    features.samplerAnisotropy = VK_TRUE;

    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
                    .setEnabledExtensionCount(static_cast<uint32_t>(requiredDeviceExtensions.size()))
                    .setPpEnabledExtensionNames(requiredDeviceExtensions.data())
                    .setPEnabledFeatures(&features);

    device = physicalDevice.createDevice(deviceCreateInfo);
}

void VulkanContext::createAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.instance = vkInstance;
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    VmaVulkanFunctions vulkanFunctions = {.vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
                                          .vkGetDeviceProcAddr = &vkGetDeviceProcAddr};
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    HUAN_CORE_INFO("Vulkan Memory Allocator created! ")
}

void VulkanContext::getQueues()
{
    device.getQueue(queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
    device.getQueue(queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
    device.getQueue(queueFamilyIndices.transferFamily.value(), 0, &transferQueue);
}

void VulkanContext::createSurface()
{
    VkSurfaceKHR tempSurface;
    const auto res = glfwCreateWindowSurface(vkInstance, window, nullptr, &tempSurface);
    if (res != VK_SUCCESS)
        HUAN_CORE_BREAK("Failed to create window surface")

    surface = vk::SurfaceKHR(tempSurface);
}

void VulkanContext::createSwapchain()
{
    swapchain = Swapchain::create(globalAppSettings.width, globalAppSettings.height);
}

void VulkanContext::createDescriptorPool()
{
    vk::DescriptorPoolCreateInfo poolCreateInfo;
    // NOTE: 池大小表示描述符数量的预期值，可以理解为Capacity
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 3),
        vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 3)};

    poolCreateInfo.setPoolSizes(poolSizes).setMaxSets(
        globalAppSettings.maxFramesInFlight); // 可以分配的最大描述符集数量

    m_descriptorPool = device.createDescriptorPool(poolCreateInfo);
    if (!m_descriptorPool)
        HUAN_CORE_BREAK("Failed to create descriptor pool")

    HUAN_CORE_INFO("DescriptorSet pool created! ")
}

/**
 * 创建我们所需要的描述符
 */
void VulkanContext::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(globalAppSettings.maxFramesInFlight, m_descriptorSetLayout);

    vk::DescriptorSetAllocateInfo allocateInfo;
    allocateInfo.setDescriptorPool(m_descriptorPool)
                .setDescriptorSetCount(globalAppSettings.maxFramesInFlight)
                .setSetLayouts(layouts);

    auto sets = device.allocateDescriptorSets(allocateInfo);
    // NOTE: 所有的渲染帧使用相同的Image 资源
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
             .setImageView((*m_textureImage->getViews().begin())->getHandle())
             .setSampler(m_textureSampler);

    for (uint32_t i = 0; i < globalAppSettings.maxFramesInFlight; i++)
    {
        m_frameDatas[i].m_descriptorSet = sets[i];
        vk::DescriptorBufferInfo bufferInfo; // 定义 描述符绑定的 资源信息 buffer or image
        bufferInfo.setBuffer(m_frameDatas[i].m_uniformBuffer->getHandle())
                  .setOffset(0)
                  .setRange(sizeof(UniformBufferObject));

        vk::WriteDescriptorSet writeBufferInfo;
        writeBufferInfo.setDstSet(m_frameDatas[i].m_descriptorSet)
                       .setDstBinding(0)
                       .setDstArrayElement(0)
                       .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                       .setDescriptorCount(1)
                       .setBufferInfo(bufferInfo);
        vk::WriteDescriptorSet imageWriteInfo;
        imageWriteInfo.setDstSet(m_frameDatas[i].m_descriptorSet)
                      .setDstBinding(1)
                      .setDstArrayElement(0)
                      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                      .setDescriptorCount(1)
                      .setImageInfo(imageInfo);

        device.updateDescriptorSets({writeBufferInfo, imageWriteInfo}, nullptr);
    }

    HUAN_CORE_INFO("DescriptorSet in frameData created and configured! ")
}

void VulkanContext::createGraphicsPipeline()
{
    HUAN_CORE_INFO("Creating graphics pipeline...")
    // Shaders
    // auto vertexShader = utils::loadFile("../../../../assets/Shaders/ModelsLoad/shader.vert.spv");
    // auto fragShader = utils::loadFile("../../../../assets/Shaders/ModelsLoad/shader.frag.spv");

    auto vsSrc = runtime::vulkan::ShaderSource("../../../../assets/Shaders/ModelsLoad/shader.vert");
    auto vsModule = runtime::vulkan::ShaderModule{device, vk::ShaderStageFlagBits::eVertex, vsSrc, "main", {}};
    auto fsSrc = runtime::vulkan::ShaderSource("../../../../assets/Shaders/ModelsLoad/shader.frag");
    auto fsModule = runtime::vulkan::ShaderModule{device, vk::ShaderStageFlagBits::eFragment, fsSrc, "main", {}};

    vk::PipelineShaderStageCreateInfo vertexShaderStageInfo;
    vertexShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex).setModule(vsModule.getHandle()).setPName(
        vsModule.getEntryPoint().c_str());
    vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment).setModule(fsModule.getHandle()).setPName(
        fsModule.getEntryPoint().c_str());
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragShaderStageInfo};

    // Dynamic states
    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicState;
    dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
                .setPDynamicStates(dynamicStates.data());
    // Input state
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    auto bindingDescriptions = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.setVertexBindingDescriptions(bindingDescriptions)
                   .setVertexAttributeDescriptions(attributeDescriptions);

    // Input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(false);
    // Viewport and scissor
    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.setViewportCount(1).setScissorCount(1);
    viewportState.setPViewports(&swapchain->getViewport()).setPScissors(&swapchain->getScissor());

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
    rasterizerInfo.setDepthClampEnable(false)
                  .setRasterizerDiscardEnable(false)
                  .setPolygonMode(vk::PolygonMode::eFill)
                  .setCullMode(vk::CullModeFlagBits::eBack)
                  .setFrontFace(vk::FrontFace::eCounterClockwise)
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
    colorBlendAttachment
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
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

    // 深度与模板缓冲
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.setDepthTestEnable(true)
                    .setDepthWriteEnable(true)
                    .setDepthCompareOp(vk::CompareOp::eLess)
                    .setDepthBoundsTestEnable(false)
                    .setMinDepthBounds(0.0f)
                    .setMaxDepthBounds(1.0f)
                    .setStencilTestEnable(false);

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo
        .setSetLayouts(m_descriptorSetLayout) // 指定 描述符集 告诉该管线预期使用哪些描述符集
        .setPushConstantRanges(nullptr);

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
                .setPDepthStencilState(&depthStencilInfo)
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

    // device.destroyShaderModule(vertexShaderModule);
    // device.destroyShaderModule(fragShaderModule);
}

/**
 * Descriptor is a way to let shader access resources in buffers freely.
 * And the DescriptorSetLayout is a descriptor set's layout.
 */
void VulkanContext::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.setBinding(0)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eVertex) // 定义这个ubo会在vertex stage使用
                    .setPImmutableSamplers(nullptr);
    vk::DescriptorSetLayoutBinding combinedImageSamplerBinding;
    combinedImageSamplerBinding.setBinding(1)
                               .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                               .setDescriptorCount(1)
                               .setStageFlags(vk::ShaderStageFlagBits::eFragment)
                               .setPImmutableSamplers(nullptr);

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, combinedImageSamplerBinding};

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.setBindings(bindings);

    m_descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
    if (!m_descriptorSetLayout)
        HUAN_CORE_BREAK("Failed to create descriptor set layout")
}

/**
 * Create RenderPass, telling the render pipeline how many color and depth buffers to use, how many samples to use for
 * them and how their contents should be handled through the pipeline.
 */
void VulkanContext::createRenderPass()
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
                   // the format of framebuffer or texture. Initial layout specifies the layout the image will have before the
                   // render pass begin.
                   .setFinalLayout(vk::ImageLayout::ePresentSrcKHR); // Auto transition to when the RenderPass ends.
    // NOTE: we don't care about the frame buffer's layout before the render pass begins, because we will clear it when
    // load. But after the render pass ends, we want to transition the image to the layout that is optimal for
    // presentation to the screen.

    vk::AttachmentReference colorAttachmentRef = {};
    // set the attachment index in attachment descriptions
    /* The index of the attachment in this array is directly referenced from the fragment shader with the
     * layout(location = 0) out vec4 outColor directive */
    colorAttachmentRef.setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);
    // 在RenderPass中使用深度 缓冲
    vk::AttachmentDescription depthAttachment;
    depthAttachment.setFormat(findDepthFormat())
                   .setSamples(vk::SampleCountFlagBits::e1)
                   .setLoadOp(vk::AttachmentLoadOp::eClear)
                   .setStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                   .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                   .setInitialLayout(vk::ImageLayout::eUndefined)
                   .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
    vk::AttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
           .setColorAttachments(colorAttachmentRef)
           .setPDepthStencilAttachment(&depthAttachmentRef); // subpass 只能使用一个深度附件

    vk::SubpassDependency subpassDependency;
    // this dependency is used to make sure that the color attachment is ready before the beginning of the subpass.
    // The VK_SUBPASS_EXTERNAL constant is used to indicate the external subpasses, which are the subpasses that are not
    // part of the render pass. And if the dst is 0, it means before the subpass. So if src is 0, it means after the
    // subpasses.
    subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
                     .setDstSubpass(0)
                     .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                      vk::PipelineStageFlagBits::eEarlyFragmentTests)
                     .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                      vk::PipelineStageFlagBits::eEarlyFragmentTests)
                     .setSrcAccessMask(vk::AccessFlagBits::eNone)
                     .setDstAccessMask(
                         vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
    std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.setAttachments(attachments).setSubpasses(subpass).setDependencies(subpassDependency);

    m_renderPass = device.createRenderPass(renderPassInfo);
    if (!m_renderPass)
        HUAN_CORE_BREAK("Failed to create render pass.")
}

void VulkanContext::createFramebuffers()
{
    m_swapchainFramebuffers.resize(swapchain->m_imageViews.size());
    // Why I should use swapchain->m_imageViews[i], instead of m_image?
    // Because the imageView essentially is a wrapper of the correspond image in the swapchain. (or a super set).
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.setRenderPass(m_renderPass)
                   .setWidth(swapchain->m_info.extent.width)
                   .setHeight(swapchain->m_info.extent.height)
                   .setLayers(1); // The number of layers of the imageView.
    for (size_t i = 0; i < swapchain->m_imageViews.size(); i++)
    {
        std::array attachments = {swapchain->m_imageViews[i], (*m_depthImage->getViews().begin())->getHandle()};
        framebufferInfo.setAttachments(attachments);

        m_swapchainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
        if (!m_swapchainFramebuffers[i])
            HUAN_CORE_BREAK("Failed to create framebuffer")
    }
}

vk::Format VulkanContext::findSupportedFormat(const std::vector<vk::Format>& candidates,
                                                         vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (auto format : candidates)
    {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    HUAN_CORE_BREAK("Failed to find supported format!")
    return vk::Format::eUndefined;
}

vk::Format VulkanContext::findDepthFormat()
{
    return findSupportedFormat({vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                               vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool VulkanContext::hasStencilComponent(const vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void VulkanContext::createDepthResources()
{
    vk::Format depthFormat = findDepthFormat();
    // TODO: deviceLocal但是直接Dynamic 可能会导致错误 待修复
    m_depthImage = runtime::ResourceSystem::getInstance()->createImage(
        vk::ImageType::e2D, vk::Extent3D(swapchain->m_info.extent.width, swapchain->m_info.extent.height, 1), 1,
        depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
    auto view = runtime::ResourceSystem::createImageView(*m_depthImage, vk::ImageViewType::e2D, depthFormat,
                                                         0);
    runtime::ResourceSystem::transitionImageLayout(
        m_depthImage->getHandle(), depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal);
}

void VulkanContext::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    vk::DeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels)
        HUAN_CORE_BREAK("Failed to load texture image! ")

    m_textureImage = runtime::ResourceSystem::getInstance()->createImage(
        vk::ImageType::e2D, vk::Extent3D(texWidth, texHeight, 1), 1, vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        pixels);

    stbi_image_free(pixels);
    runtime::ResourceSystem::createImageView(*m_textureImage, vk::ImageViewType::e2D,
                                             vk::Format::eR8G8B8A8Srgb,
                                             0);
}

void VulkanContext::createTextureSampler()
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.setMagFilter(vk::Filter::eLinear)
               .setMinFilter(vk::Filter::eLinear)
               .setAddressModeU(vk::SamplerAddressMode::eRepeat)
               .setAddressModeV(vk::SamplerAddressMode::eRepeat)
               .setAddressModeW(vk::SamplerAddressMode::eRepeat)
               .setAnisotropyEnable(VK_TRUE)
               .setMaxAnisotropy(physicalDevice.getProperties().limits.maxSamplerAnisotropy)
               .setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
               .setUnnormalizedCoordinates(VK_FALSE)
               .setCompareEnable(VK_FALSE)
               .setCompareOp(vk::CompareOp::eAlways)
               .setMipmapMode(vk::SamplerMipmapMode::eLinear)
               .setMipLodBias(0.0f)
               .setMinLod(0.0f)
               .setMaxLod(0.0f);

    m_textureSampler = device.createSampler(samplerInfo);
}

void VulkanContext::loadModel()
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
        HUAN_CORE_BREAK("Failed to load model! {}", warn + err);

    std::unordered_map<uint32_t, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.m_pos = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]};

            vertex.m_texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],
                                 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.m_color = {1.0f, 1.0f, 1.0f};
            // if (!uniqueVertices.contains(index.vertex_index))
            // {
            //     uniqueVertices[index.vertex_index] = m_vertices.size();
            //     m_vertices.push_back(vertex);
            // }

            m_vertices.push_back(vertex);
            m_indices.push_back(m_indices.size());
        }
    }
    HUAN_CORE_TRACE("Model vertex num: {}", m_vertices.size())
}

void VulkanContext::drawFrame()
{
    auto& curInFlightFence = m_frameDatas[m_currentFrame].m_fence;
    auto& curImageAvailableSemaphore = m_frameDatas[m_currentFrame].m_imageAvailableSemaphore;
    auto& curRenderFinishedSemaphore = m_frameDatas[m_currentFrame].m_renderFinishedSemaphore;
    auto& curCommandBuffer = m_frameDatas[m_currentFrame].m_commandBuffer;

    auto resWaitFences = device.waitForFences(curInFlightFence, true, UINT64_MAX);

    if (resWaitFences != vk::Result::eSuccess)
        HUAN_CORE_ERROR("Failed to wait for fences")

    uint32_t imageIndex;
    auto resAcqNextImage =
        swapchain->acquireNextImage(UINT64_MAX, curImageAvailableSemaphore, VK_NULL_HANDLE, imageIndex);
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

    updateUniformBuffer();

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
    presentInfo.setSwapchains(swapchain->m_swapchain).setImageIndices(imageIndex);

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

/**
 * 使用各自独立的队列
 */
void VulkanContext::queryQueueFamilyIndices()
{
    const auto properties = physicalDevice.getQueueFamilyProperties();
    for (auto i = 0; i < properties.size(); i++)
    {
        if (!queueFamilyIndices.graphicsFamily.has_value() && properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            queueFamilyIndices.graphicsFamily = i;
        }
        else if (!queueFamilyIndices.presentFamily.has_value() && physicalDevice.getSurfaceSupportKHR(i, surface))
        {
            queueFamilyIndices.presentFamily = i;
        }
        else if (!queueFamilyIndices.transferFamily.has_value() &&
                 properties[i].queueFlags & vk::QueueFlagBits::eTransfer)
        {
            queueFamilyIndices.transferFamily = i;
        }

        if (queueFamilyIndices.isComplete())
            break;
    }
    if (!queueFamilyIndices.isComplete())
        HUAN_CORE_BREAK("Failed to find a queue family that supports both graphics and presentation")
}

void VulkanContext::initVulkan()
{
    createInstance();
    createDebugMessenger();

    createSurface();

    pickPhysicalDevice();
    queryQueueFamilyIndices();
    createDevice();
    createAllocator();
    getQueues();

    createSwapchain();

    createRenderPass();

    createDescriptorSetLayout();
    createDescriptorPool();
    createCommandPool();

    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();

    createTextureImage();
    createTextureSampler();
    loadModel();
    createVertexBufferAndMemory();
    createIndexBufferAndMemory();

    // Create CommandBuffer and Sync objects for per frame
    createFrameData();

    HUAN_CORE_TRACE(R"(

    __  __                     ____                 __
   / / / /_  ______ _____     / __ \___  ____  ____/ /__  ________  _____
  / /_/ / / / / __ `/ __ \   / /_/ / _ \/ __ \/ __  / _ \/ ___/ _ \/ ___/
 / __  / /_/ / /_/ / / / /  / _, _/  __/ / / / /_/ /  __/ /  /  __/ /
/_/ /_/\__,_/\__,_/_/ /_/  /_/ |_|\___/_/ /_/\__,_/\___/_/   \___/_/

                                                                               )");
}

void VulkanContext::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        drawFrame();
        glfwPollEvents();
    }
}

void VulkanContext::updateUniformBuffer()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    UniformBufferObject ubo;
    auto& curUniformBuffer = m_frameDatas[m_currentFrame].m_uniformBuffer;

    ubo.m_model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.m_proj = glm::perspective(
        glm::radians(45.0f), swapchain->m_info.extent.width / (float)swapchain->m_info.extent.height, 0.1f, 100.0f);
    ubo.m_proj[1][1] *= -1;
    ubo.m_view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    curUniformBuffer->updateDirectly(static_cast<void*>(&ubo), sizeof(ubo), 0);
}

void VulkanContext::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setPInheritanceInfo(nullptr);

    if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess)
        HUAN_CORE_BREAK("Failed to begin recording command buffer.")

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.setRenderPass(m_renderPass)
                  .setFramebuffer(m_swapchainFramebuffers[imageIndex])
                  .setRenderArea(vk::Rect2D{{}, swapchain->m_info.extent});
    // 联合体 NOTE: clearValues中的顺序应当和AttachmentDescription的顺序一致
    std::array<vk::ClearValue, 2> clearValues = {{{{129.0 / 255.0, 216.0 / 255.0, 207.0 / 255.0, 1.0f}}, {{1, 0}}}};

    renderPassInfo.setClearValues(clearValues);
    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);
    vk::Viewport viewport{0, 0, (float)swapchain->m_info.extent.width, (float)swapchain->m_info.extent.height, 0, 1};
    commandBuffer.setViewport(0, 1, &viewport);
    vk::Rect2D scissor{{0, 0}, swapchain->m_info.extent};
    commandBuffer.setScissor(0, 1, &scissor);

    vk::Buffer vertexBuffers[] = {m_vertexBuffer->getHandle()};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(m_indexBuffer->getHandle(), 0, vk::IndexType::eUint32);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1,
                                     &m_frameDatas[m_currentFrame].m_descriptorSet, 0, nullptr);

    // commandBuffer.draw(m_vertices.size(), 1, 0, 0);

    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);
    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void VulkanContext::recreateSwapchain()
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
    for (auto& view : m_depthImage->getViews())
    {
        delete view;
    };
    m_depthImage.reset();

    // Create
    createSwapchain();
    createDepthResources();
    createFramebuffers();
    m_framebufferResized = false;
}

void VulkanContext::cleanup()
{
    HUAN_CORE_INFO("Cleaning up...\n\n")
    device.waitIdle();

    for (auto& frameData : m_frameDatas)
    {
        device.destroyFence(frameData.m_fence);
        device.destroySemaphore(frameData.m_renderFinishedSemaphore);
        device.destroySemaphore(frameData.m_imageAvailableSemaphore);
        frameData.m_uniformBuffer.reset();
    }
    HUAN_CORE_INFO("FrameDatas destroyed.")

    device.destroyCommandPool(m_commandPool);
    device.destroyCommandPool(m_transferCommandPool);
    HUAN_CORE_INFO("CommandPool destroyed.")
    for (auto& view : m_depthImage->getViews())
    {
        delete view;
    };
    m_depthImage.reset();
    HUAN_CORE_INFO("Depth image and view destroyed.")
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
    device.destroyDescriptorPool(m_descriptorPool);
    HUAN_CORE_INFO("DescriptorPool destroyed.")
    device.destroyDescriptorSetLayout(m_descriptorSetLayout);
    HUAN_CORE_INFO("DescriptorSet layout destroyed. ")
    vkInstance.destroySurfaceKHR(surface);
    HUAN_CORE_INFO("Surface destroyed.")
    device.destroySampler(m_textureSampler);
    HUAN_CORE_INFO("Sampler destroyed.")
    for (auto& view : m_textureImage->getViews())
    {
        delete view;
    };
    m_textureImage.reset();
    HUAN_CORE_INFO("m_textureImage and view freed! ")
    m_vertexBuffer.reset();
    HUAN_CORE_INFO("VertexBuffer and VertexBuffer's memory freed! ")
    m_indexBuffer.reset();
    HUAN_CORE_INFO("IndexBuffer and IndexBuffer's memory freed! ")
    vmaDestroyAllocator(allocator);
    HUAN_CORE_INFO("Allocator destroyed.")
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

VulkanContext::~VulkanContext()
{
    // Do nothing
}
} // namespace huan