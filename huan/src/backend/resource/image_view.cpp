//
// Created by TU on 2025/8/11.
//

#include "huan/backend/resource/image_view.hpp"

#include "huan/HelloTriangleApplication.hpp"
#include "huan/backend/resource/vulkan_image.hpp"
#include <vulkan/vulkan_format_traits.hpp>

namespace huan::runtime::vulkan
{
ImageView::ImageView(const Image& image, vk::ImageViewType viewType, vk::Format format, uint32_t baseMipLevel,
                     uint32_t baseArrayLayer, uint32_t nMipLevels, uint32_t nArrayLayers)
    : ParentType(image.getDeviceHandle())
{
    if (format == vk::Format::eUndefined)
        m_format = image.getFormat();

    m_subresourceRange = vk::ImageSubresourceRange{
        .aspectMask = (std::string(vk::componentName(format, 0)) == "D")
                          ? vk::ImageAspectFlagBits::eDepth
                          : vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = baseMipLevel,
        .levelCount = nMipLevels == 0 ? m_image->getSubresource().mipLevel : nMipLevels,
        .baseArrayLayer = baseArrayLayer,
        .layerCount = nArrayLayers == 0 ? m_image->getSubresource().arrayLayer : nArrayLayers
    };
    vk::ImageViewCreateInfo info{
        .image = m_image,
        .viewType = viewType,
        .format = format,
        .subresourceRange = m_subresourceRange
    };
    setHandle(HelloTriangleApplication::getInstance()->device.createImageView(info));
    m_image->getViews().emplace(this);
}

ImageView::ImageView(ImageView&& that) noexcept
    : ParentType(std::move(that)), m_image(std::move(that.m_image)), m_format(std::move(that.m_format)),
      m_subresourceRange(std::move(that.m_subresourceRange))
{
    auto& views = m_image->getViews();
    views.erase(&that);
    views.emplace(this);
    that.setHandle(nullptr);
}

ImageView::~ImageView()
{
    if (getHandle())
    {
        getDeviceHandle().destroyImageView(getHandle());
    }
}

vk::Format ImageView::getFormat() const
{
    return m_format;
}

const Image& ImageView::getImage() const
{
    return *m_image;
}

void ImageView::setImage(Image& image)
{
    m_image = &image;
}

vk::ImageSubresourceLayers ImageView::getSubresourceLayers() const
{
    return vk::ImageSubresourceLayers{
        .aspectMask = m_subresourceRange.aspectMask,
        .baseMipLevel = m_subresourceRange.baseMipLevel,
        .baseArrayLayer = m_subresourceRange.baseArrayLayer,
        .layerCount = m_subresourceRange.layerCount
    };
}

vk::ImageSubresourceRange ImageView::getSubresourceRange() const
{
    return m_subresourceRange;
}
}