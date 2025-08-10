//
// Created by 86156 on 4/4/2025.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "vk_mem_alloc.h"

#include <huan/common.hpp>
#include <huan/backend/swapchain.hpp>

const std::string MODEL_PATH = "../../../../assets/Models/viking_room/viking_room.obj";
const std::string TEXTURE_PATH = "../../../../assets/Models/viking_room/viking_room.png";

struct GLFWwindow;

namespace huan
{
namespace vulkan
{
class Image;
class Buffer;
} // namespace vulkan

struct Vertex
{
    glm::vec3 m_pos;
    glm::vec3 m_color;
    glm::vec2 m_texCoord;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription;
        bindingDescription.setBinding(0).setStride(sizeof(Vertex)).setInputRate(vk::VertexInputRate::eVertex);

        return bindingDescription;
    }

    static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, m_pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, m_color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[2].offset = offsetof(Vertex, m_texCoord);

        return attributeDescriptions;
    }
};

struct VulkanFrameData
{
    vk::CommandBuffer m_commandBuffer;
    vk::Fence m_fence;
    vk::Semaphore m_imageAvailableSemaphore;
    vk::Semaphore m_renderFinishedSemaphore;

    Scope<vulkan::Buffer> m_uniformBuffer;
    vk::DescriptorSet m_descriptorSet;
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 m_model;
    alignas(16) glm::mat4 m_view;
    alignas(16) glm::mat4 m_proj;
};

class HUAN_API HelloTriangleApplication
{
public:
    static HelloTriangleApplication* getInstance()
    {
        if (!instance)
        {
            instance = new HelloTriangleApplication();
        }
        return instance;
    };

    void init();
    void run();
    void cleanup();
    void mainLoop();
    void updateUniformBuffer();
    void drawFrame();

    HelloTriangleApplication();
    ~HelloTriangleApplication();

    [[nodiscard]] bool isInitialized() const
    {
        return initialized;
    }

private:
    void initLogSystem();
    void initWindow();

    void initVulkan();
    [[nodiscard]] static std::vector<const char*> getRequiredInstanceExtensions();
    [[nodiscard]] static std::vector<const char*> getRequiredDeviceExtensions();
    void createInstance();
    void createDebugMessenger();
    void pickPhysicalDevice();
    void queryQueueFamilyIndices();
    void createDevice();
    void createAllocator();
    void getQueues();

    void createSurface();
    void createSwapchain();
    void createDescriptorPool();
    void createDescriptorSets();
    void createGraphicsPipeline();
    void createDescriptorSetLayout();
    void createRenderPass();
    void createFramebuffers();
    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                   vk::FormatFeatureFlags features);
    vk::Format findDepthFormat();
    bool hasStencilComponent(vk::Format format);

    void createDepthResources();
    void createTextureImage();
    void createTextureSampler();
    void loadModel();
    void createVertexBufferAndMemory();
    void createIndexBufferAndMemory();
    void createCommandPool();

    void createCommandBuffer(); // Using in createFrameData()
    void createSynchronization();
    void createUniformBuffers();
    void createFrameData();

    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
    void recreateSwapchain();

public:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> computeFamily;

        bool isComplete() const
        {
            return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
        }
    };

INNER_VISIBLE:
    inline static HelloTriangleApplication* instance = nullptr;
    GLFWwindow* window = nullptr;
    vk::Instance vkInstance;
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    VmaAllocator allocator;
    QueueFamilyIndices queueFamilyIndices;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Queue transferQueue;

    vk::SurfaceKHR surface;
    Scope<Swapchain> swapchain;

    vk::PipelineLayout m_pipelineLayout;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::DescriptorPool m_descriptorPool;
    vk::Pipeline m_graphicsPipeline;
    vk::RenderPass m_renderPass;

    // 画架们
    std::vector<vk::Framebuffer> m_swapchainFramebuffers;

    vk::CommandPool m_commandPool;
    vk::CommandPool m_transferCommandPool;

    std::vector<VulkanFrameData> m_frameDatas;
    uint32_t m_currentFrame = 0;

    Scope<vulkan::Buffer> m_vertexBuffer;
    Scope<vulkan::Buffer> m_indexBuffer;

    Scope<vulkan::Image> m_textureImage;
    vk::Sampler m_textureSampler;

    Scope<vulkan::Image> m_depthImage;


    bool initialized = false;
    bool m_framebufferResized = false;

    // 顶点数据
    std::vector<Vertex> m_vertices = {};

    std::vector<uint32_t> m_indices = {};
};
} // namespace huan
