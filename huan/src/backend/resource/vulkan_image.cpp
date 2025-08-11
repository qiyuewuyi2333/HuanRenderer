#include "huan/backend/resource/vulkan_image.hpp"

#include "huan/backend/resource/image_view.hpp"
#include "huan/backend/resource/resource_system.hpp"

#include <stdexcept>

namespace huan::runtime::vulkan
{
/**
 * @brief 根据图像范围自动推断图像类型
 */
vk::ImageType findImageType(const vk::Extent3D& extent)
{
    uint32_t dimensionCount = 0;

    if (extent.width >= 1)
    {
        dimensionCount++;
    }

    if (extent.height >= 1)
    {
        dimensionCount++;
    }

    if (extent.depth > 1)
    {
        dimensionCount++;
    }

    switch (dimensionCount)
    {
    case 1:
        return vk::ImageType::e1D;
    case 2:
        return vk::ImageType::e2D;
    case 3:
        return vk::ImageType::e3D;
    default:
        throw std::runtime_error("无法确定图像类型：无效的范围");
    }
}

#pragma region ImageBuilder
ImageBuilder::ImageBuilder(VmaAllocator allocator, const vk::Extent3D& extent)
    : ParentType(allocator,
                 vk::ImageCreateInfo{})
{
    auto& createInfo = getCreateInfo();
    createInfo.setExtent(extent)
              .setArrayLayers(1)
              .setMipLevels(1)
              .setImageType(findImageType(extent))
              .setFormat(vk::Format::eR8G8B8A8Unorm)
              .setSamples(vk::SampleCountFlagBits::e1);
}

ImageBuilder::ImageBuilder(VmaAllocator allocator, uint32_t width, uint32_t height, uint32_t depth) :
    ImageBuilder(allocator, vk::Extent3D{width, height, depth})
{
}

ImageBuilder& ImageBuilder::setFormat(vk::Format format)
{
    getCreateInfo().setFormat(format);
    return *this;
}

ImageBuilder& ImageBuilder::setUsage(vk::ImageUsageFlags usage)
{
    getCreateInfo().setUsage(usage);
    return *this;
}

ImageBuilder& ImageBuilder::setFlags(vk::ImageCreateFlags flags)
{
    getCreateInfo().setFlags(flags);
    return *this;
}

ImageBuilder& ImageBuilder::setImageType(vk::ImageType type)
{
    getCreateInfo().setImageType(type);
    return *this;
}

ImageBuilder& ImageBuilder::setArrayLayers(uint32_t layers)
{
    getCreateInfo().setArrayLayers(layers);
    return *this;
}

ImageBuilder& ImageBuilder::setMipLevels(uint32_t levels)
{
    getCreateInfo().setMipLevels(levels);
    return *this;
}

ImageBuilder& ImageBuilder::setSampleCount(vk::SampleCountFlagBits sampleCount)
{
    getCreateInfo().setSamples(sampleCount);
    return *this;
}

ImageBuilder& ImageBuilder::setTiling(vk::ImageTiling tiling)
{
    getCreateInfo().setTiling(tiling);
    return *this;
}

ImageBuilder& ImageBuilder::setSharingMode(vk::SharingMode sharingMode)
{
    getCreateInfo().setSharingMode(sharingMode);
    return *this;
}

ImageBuilder& ImageBuilder::setQueueFamilyIndices(const std::vector<uint32_t>& queueFamilyIndices)
{
    getCreateInfo().setQueueFamilyIndices(queueFamilyIndices);
    return *this;
}

ImageBuilder& ImageBuilder::setInitialLayout(vk::ImageLayout initialLayout)
{
    getCreateInfo().setInitialLayout(initialLayout);
    return *this;
}

Image ImageBuilder::build(vk::Device& device) const
{
    return Image(device, *this);
}

Scope<Image> ImageBuilder::buildUnique(vk::Device& device) const
{
    return createScope<Image>(device, *this);
}
#pragma endregion


#pragma region VulkanImage
Image::Image(vk::Device& device,
             vk::Image handle,
             const vk::Extent3D& extent,
             vk::Format format,
             vk::ImageUsageFlags imageUsage,
             vk::SampleCountFlagBits sampleCount)
    : ParentType(device, handle)
{
    m_createInfo.sType = vk::StructureType::eImageCreateInfo;
    m_createInfo.extent = extent;
    m_createInfo.imageType = findImageType(extent);
    m_createInfo.format = format;
    m_createInfo.samples = sampleCount;
    m_createInfo.usage = imageUsage;
    m_createInfo.mipLevels = 1;
    m_createInfo.arrayLayers = 1;
    m_createInfo.tiling = vk::ImageTiling::eOptimal;
    m_createInfo.initialLayout = vk::ImageLayout::eUndefined;
    m_createInfo.sharingMode = vk::SharingMode::eExclusive;

    // 设置子资源信息
    m_subresource.mipLevel = 1;
    m_subresource.arrayLayer = 1;

    // 根据使用标志设置适当的方面掩码
    if (imageUsage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
    {
        m_subresource.aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (format == vk::Format::eD32SfloatS8Uint ||
            format == vk::Format::eD24UnormS8Uint)
        {
            m_subresource.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    }
    else
    {
        m_subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    }
}

Image::Image(vk::Device& device, const ImageBuilder& builder)
    : ParentType(device, builder.getAllocator(), builder.getAllocationCreateInfo())
      , m_createInfo(builder.getCreateInfo())
{
    this->setHandle(this->createImage(builder.getCreateInfo()));
    this->m_subresource.arrayLayer = m_createInfo.arrayLayers;
    this->m_subresource.mipLevel = m_createInfo.mipLevels;

#ifdef HUAN_DEBUG
    if (!builder.getDebugName().empty())
    {
        this->setDebugName(builder.getDebugName());
    }
#endif

}


Image::Image(Image&& other) noexcept
    : ParentType(std::move(other)), m_createInfo(std::move(other.m_createInfo)),
      m_subresource(std::move(other.m_subresource)),
      m_views(std::move(other.m_views))
{
    for (auto& view : other.m_views)
    {
        view->setImage(*this);
    }
}

Image::~Image()
{
    destroyImage(getHandle());
}

vk::ImageType Image::getType() const
{
    return m_createInfo.imageType;
}

const vk::Extent3D& Image::getExtent() const
{
    return m_createInfo.extent;
}

vk::Format Image::getFormat() const
{
    return m_createInfo.format;
}

vk::SampleCountFlagBits Image::getSampleCount() const
{
    return m_createInfo.samples;
}

vk::ImageUsageFlags Image::getUsage() const
{
    return m_createInfo.usage;
}

vk::ImageTiling Image::getTiling() const
{
    return m_createInfo.tiling;
}

uint32_t Image::getArrayLayerCount() const
{
    return m_createInfo.arrayLayers;
}

const vk::ImageSubresource& Image::getSubresource() const
{
    return m_subresource;
}

std::unordered_set<ImageView*>& Image::getViews()
{
    return m_views;
}

uint8_t* Image::map()
{
    if (m_createInfo.tiling != vk::ImageTiling::eLinear)
    {
        HUAN_CORE_WARN("Mapping image memory that is not linear");
    }
    return ParentType::map();
}

#pragma endregion
} // namespace huan::runtime::vulkan