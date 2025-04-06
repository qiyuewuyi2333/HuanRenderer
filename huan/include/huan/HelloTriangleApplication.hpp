//
// Created by 86156 on 4/4/2025.
//

#ifndef HELLOTRIANGLEAPPLICATION_HPP
#define HELLOTRIANGLEAPPLICATION_HPP

#include <optional>

#include "huan/common.hpp"
#include "vulkan/vulkan.hpp"
#include <huan/backend/swapchain.hpp>


struct GLFWwindow;

namespace huan
{
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
        [[nodiscard]] bool isInitialized() const
        {
            return initialized;
        }
        void run();

        void cleanup();
        
        HelloTriangleApplication();
        ~HelloTriangleApplication();
    private:
        void initLogSystem();
        void initWindow();
        void createCommandPool();
        void createCommandBuffer();
        void createSynchronization();
        void initVulkan();
        std::vector<const char*> getRequiredInstanceExtensions();
        std::vector<const char*> getRequiredDeviceExtensions();
        void createInstance();
        void createDebugMessenger();
        void pickPhysicalDevice();
        void queryQueueFamilyIndices();
        void createDevice();
        void getQueues();

        void createSurface();
        void createSwapchain();
        void createGraphicsPipeline();
        void createRenderPass();
        void createFramebuffers();

        void drawFrame();
        void mainLoop();
        void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);


    public:
        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;
            std::optional<uint32_t> computeFamily;
            std::optional<uint32_t> transferFamily;

            bool isComplete() const
            {
                return graphicsFamily.has_value() && presentFamily.has_value();
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
        QueueFamilyIndices queueFamilyIndices;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        vk::SurfaceKHR surface;
        Scope<Swapchain> swapchain;
        
        vk::PipelineLayout m_pipelineLayout;
        vk::Pipeline m_graphicsPipeline;
        vk::RenderPass m_renderPass;

        // 画架们
        std::vector<vk::Framebuffer> m_swapchainFramebuffers;

        vk::CommandPool m_commandPool;
        vk::CommandBuffer m_commandBuffer;

        vk::Semaphore m_imageAvailableSemaphore;
        vk::Semaphore m_renderFinishedSemaphore;
        vk::Fence m_inFlightFence;
        

        bool initialized = false;
    };
}


#endif //HELLOTRIANGLEAPPLICATION_HPP
