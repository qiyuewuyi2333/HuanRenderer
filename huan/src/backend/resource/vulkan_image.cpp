#include "huan/backend/resource/vulkan_image.hpp"

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
    m_subresource.mipLevel = 0;
    m_subresource.arrayLayer = 0;

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

uint32_t Image::getMipLevels() const
{
    return m_createInfo.mipLevels;
}

const vk::ImageSubresource& Image::getSubresource() const
{
    return m_subresource;
}

std::unordered_set<VulkanImageView*>& Image::getViews()
{
    return m_views;
}

const std::unordered_set<VulkanImageView*>& Image::getViews() const
{
    return m_views;
}

bool Image::isSwapchainImage() const
{
    return isSwapchainImage_;
}

void Image::initialize()
{
    // 设置调试名称（如果有）
    if (!debugName_.empty())
    {
        setDebugName(debugName_);
    }

    // 可以在这里添加其他初始化逻辑
    // 比如设置初始布局、执行初始转换等
}

void Image::cleanup()
{
    // 通知所有关联的图像视图即将销毁
    for (auto* view : m_views)
    {
        if (view)
        {
            view->onImageDestroying();
        }
    }
    m_views.clear();

    // 销毁 Vulkan 资源
    if (image_ && !isSwapchainImage_)
    {
        device_.getHandle().destroyImage(image_);
        image_ = nullptr;
    }

    if (memory_ && ownsMemory_)
    {
        device_.getHandle().freeMemory(memory_);
        memory_ = nullptr;
    }
}

vk::DeviceSize Image::getRequiredMemorySize() const
{
    if (image_)
    {
        auto memoryRequirements = device_.getHandle().getImageMemoryRequirements(image_);
        return memoryRequirements.size;
    }
    return 0;
}

bool Image::hasUsage(vk::ImageUsageFlags usage) const
{
    return static_cast<bool>(m_createInfo.usage & usage);
}

bool Image::isCompatibleWith(const Image& other) const
{
    return m_createInfo.format == other.m_createInfo.format &&
           m_createInfo.extent.width == other.m_createInfo.extent.width &&
           m_createInfo.extent.height == other.m_createInfo.extent.height &&
           m_createInfo.extent.depth == other.m_createInfo.extent.depth &&
           m_createInfo.samples == other.m_createInfo.samples &&
           m_createInfo.arrayLayers == other.m_createInfo.arrayLayers &&
           m_createInfo.mipLevels == other.m_createInfo.mipLevels;
}

void Image::setDebugName(const std::string& name)
{
    debugName_ = name;

    // 设置 Vulkan 对象的调试名称（如果支持调试扩展）
    if (image_&& device_

    
    .
    isDebugUtilsEnabled()
    )
    {
        vk::DebugUtilsObjectNameInfoEXT nameInfo{
            .objectType = vk::ObjectType::eImage,
            .objectHandle = reinterpret_cast<uint64_t>(static_cast<VkImage>(image_)),
            .pObjectName = debugName_.c_str()
        };

        try
        {
            device_.getHandle().setDebugUtilsObjectNameEXT(nameInfo);
        }
        catch (const std::exception& e)
        {
            // 调试名称设置失败不应该中断程序
            // 可以记录警告日志
        }
    }
}

std::string Image::getFormatString() const
{
    return vk::to_string(m_createInfo.format);
}

std::string Image::getUsageString() const
{
    return vk::to_string(m_createInfo.usage);
}

bool Image::isDepthStencilFormat() const
{
    return hasUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment) ||
           m_createInfo.format == vk::Format::eD16Unorm ||
           m_createInfo.format == vk::Format::eD32Sfloat ||
           m_createInfo.format == vk::Format::eD16UnormS8Uint ||
           m_createInfo.format == vk::Format::eD24UnormS8Uint ||
           m_createInfo.format == vk::Format::eD32SfloatS8Uint;
}

bool Image::isColorFormat() const
{
    return !isDepthStencilFormat();
}

uint32_t Image::calculateMipLevels() const
{
    return static_cast<uint32_t>(std::floor(std::log2(std::max(m_createInfo.extent.width, m_createInfo.extent.height))))
           +
           1;
}

vk::ImageSubresourceRange Image::getFullSubresourceRange() const
{
    return vk::ImageSubresourceRange{
        .aspectMask = m_subresource.aspectMask,
        .baseMipLevel = 0,
        .levelCount = m_createInfo.mipLevels,
        .baseArrayLayer = 0,
        .layerCount = m_createInfo.arrayLayers
    };
}

vk::ImageSubresourceRange Image::getSubresourceRange(uint32_t baseMipLevel,
                                                     uint32_t levelCount,
                                                     uint32_t baseArrayLayer,
                                                     uint32_t layerCount) const
{
    return vk::ImageSubresourceRange{
        .aspectMask = m_subresource.aspectMask,
        .baseMipLevel = baseMipLevel,
        .levelCount = levelCount == VK_REMAINING_MIP_LEVELS ? m_createInfo.mipLevels - baseMipLevel : levelCount,
        .baseArrayLayer = baseArrayLayer,
        .layerCount = layerCount == VK_REMAINING_ARRAY_LAYERS ? m_createInfo.arrayLayers - baseArrayLayer : layerCount
    };
}

void Image::addView(VulkanImageView* view)
{
    if (view)
    {
        m_views.insert(view);
    }
}

void Image::removeView(VulkanImageView* view)
{
    if (view)
    {
        m_views.erase(view);
    }
}
#pragma endregion
} // namespace huan::runtime::vulkan