//
// Created by 86156 on 4/4/2025.
//
#include "huan/backend/swapchain.hpp"

#include "huan/VulkanContext.hpp"
#include "huan/log/Log.hpp"

namespace huan
{
Scope<Swapchain> Swapchain::create(uint32_t width, uint32_t height)
{
    return createScope<Swapchain>(width, height);
}

Swapchain::Swapchain(uint32_t width, uint32_t height)
    : m_device(VulkanContext::getInstance()->device)
{
    querySwapchainSupportInfo(width, height);
    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.setClipped(true)
              .setImageArrayLayers(1)
              .setImageColorSpace(m_info.format.colorSpace)
              .setImageExtent(m_info.extent)
              .setImageFormat(m_info.format.format) // eB8G8R8A8Srgb etc.
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
              .setMinImageCount(m_info.imageCount)
              .setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
              .setPresentMode(m_info.presentMode)
              .setSurface(VulkanContext::getInstance()->surface)
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setOldSwapchain(nullptr);

    auto& queueIndices = VulkanContext::getInstance()->queueFamilyIndices;
    // NOTE: 必须将这个数组放到if的外部，否则release模式下运行会出现validation error
    const uint32_t queueFamilyIndices[] = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
    if (queueIndices.graphicsFamily.value() != queueIndices.presentFamily.value())
    {
        createInfo.setQueueFamilyIndices(queueFamilyIndices)
                  .setImageSharingMode(vk::SharingMode::eConcurrent);
    }
    else
    {
        createInfo.setQueueFamilyIndices(queueIndices.graphicsFamily.value())
                  .setImageSharingMode(vk::SharingMode::eExclusive);
    }

    m_swapchain = m_device.createSwapchainKHR(createInfo);

    getImages();
    createImageViews();

    m_viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
    m_scissor = vk::Rect2D({0, 0}, m_info.extent);
}

Swapchain::~Swapchain()
{
    for (auto& imageView : m_imageViews)
        m_device.destroyImageView(imageView);
    m_device.destroySwapchainKHR(m_swapchain);
}

vk::Result Swapchain::acquireNextImage(uint64_t timeOut, vk::Semaphore imageAvailableSemaphore,
                                       vk::Fence inFlightFence, uint32_t& imageIndex)
{
    return m_device.acquireNextImageKHR(m_swapchain, timeOut, imageAvailableSemaphore, inFlightFence, &imageIndex);
}

vk::Viewport& Swapchain::getViewport()
{
    return m_viewport;
}

vk::Rect2D& Swapchain::getScissor()
{
    return m_scissor;
}

vk::Format Swapchain::getImageFormat() const
{
    return m_info.format.format;
}

void Swapchain::querySwapchainSupportInfo(uint32_t width, uint32_t height)
{
    auto& physicalDevice = VulkanContext::getInstance()->physicalDevice;
    auto formats = physicalDevice.getSurfaceFormatsKHR(VulkanContext::getInstance()->surface);
    this->m_info.format = formats[0];
    for (const auto& format : formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            this->m_info.format = format;
            break;
        }
    }
    auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(VulkanContext::getInstance()->surface);
    m_info.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);
    m_info.extent.width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                                               capabilities.maxImageExtent.width);
    m_info.extent.height = std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                                                capabilities.maxImageExtent.height);
    m_info.transform = capabilities.currentTransform;
    m_info.presentMode = vk::PresentModeKHR::eFifo;
    for (const auto& presentMode : physicalDevice.getSurfacePresentModesKHR(
             VulkanContext::getInstance()->surface))
    {
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            m_info.presentMode = presentMode;
            break;
        }
    }
}

void Swapchain::getImages()
{
    m_images = m_device.getSwapchainImagesKHR(m_swapchain);
}

void Swapchain::createImageViews()
{
    m_imageViews.resize(m_images.size());
    for (size_t i = 0; i < m_images.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo;
        createInfo.setImage(m_images[i])
                  .setViewType(vk::ImageViewType::e2D) // 2D image
                  .setFormat(m_info.format.format)
                  .setComponents({
                      vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
                      vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity
                  }) // set component mapping, you got color component from image, and swizzle it.
                  .setSubresourceRange(
                      {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        m_imageViews[i] = m_device.createImageView(createInfo);
    }
}
}