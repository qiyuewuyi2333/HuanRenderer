//
// Created by 86156 on 4/4/2025.
//
#pragma once
#include "huan/common.hpp"
#include <vulkan/vulkan.hpp>

namespace huan
{
    class Swapchain final
    {
    public:
        static Scope<Swapchain> create(uint32_t width, uint32_t height);
        Swapchain(uint32_t width, uint32_t height);
        ~Swapchain();

        vk::Result acquireNextImage(uint64_t timeOut, vk::Semaphore imageAvailableSemaphore, vk::Fence inFlightFence, uint32_t& imageIndex) const;
        struct SwapchainSupportInfo
        {
            uint32_t imageCount;
            vk::Extent2D extent;
            vk::SurfaceFormatKHR format;
            vk::SurfaceTransformFlagsKHR transform;
            vk::PresentModeKHR presentMode;
        };
        vk::Viewport& getViewport();
        vk::Rect2D& getScissor();
        vk::Format getImageFormat() const;
    private:
        void querySwapchainSupportInfo(uint32_t width, uint32_t height);
    INNER_VISIBLE:
        vk::Device& m_device;
        vk::SwapchainKHR m_swapchain;
        SwapchainSupportInfo m_info;

        std::vector<vk::Image> m_images;
        std::vector<vk::ImageView> m_imageViews;

        vk::Viewport m_viewport;
        vk::Rect2D m_scissor;
    };
}
