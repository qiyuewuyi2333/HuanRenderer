//
// Created by TU on 2025/8/10.
//
#pragma once

#include "vulkan_allocated.hpp"
#include "vulkan_builder_base.hpp"

#include <vulkan/vulkan.hpp>
#include <unordered_set>

namespace huan::runtime::vulkan
{

/**
 * @brief VulkanImage 构建器，使用构建器模式创建图像
 */
class ImageBuilder : public BuilderBase<ImageBuilder, vk::ImageCreateInfo>
{
    using ParentType = BuilderBase<ImageBuilder, vk::ImageCreateInfo>;

public:
    explicit ImageBuilder(VmaAllocator allocator, const vk::Extent3D& extent);
    ImageBuilder(VmaAllocator allocator, uint32_t width, uint32_t height, uint32_t depth = 1);

    // 流式接口 - 返回引用以支持链式调用
    ImageBuilder& setFormat(vk::Format format);
    ImageBuilder& setUsage(vk::ImageUsageFlags usage);
    ImageBuilder& setFlags(vk::ImageCreateFlags flags);
    ImageBuilder& setImageType(vk::ImageType type);
    ImageBuilder& setArrayLayers(uint32_t layers);
    ImageBuilder& setMipLevels(uint32_t levels);
    ImageBuilder& setSampleCount(vk::SampleCountFlagBits sampleCount);
    ImageBuilder& setTiling(vk::ImageTiling tiling);
    ImageBuilder& setSharingMode(vk::SharingMode sharingMode);
    ImageBuilder& setQueueFamilyIndices(const std::vector<uint32_t>& queueFamilyIndices);
    ImageBuilder& setInitialLayout(vk::ImageLayout initialLayout);

    // 扩展支持
    template <typename ExtensionType>
        requires requires(ExtensionType ext)
        {
            { ext.pNext } -> std::same_as<const void*>;
        }
    ImageBuilder& setExtension(ExtensionType& extension);

    // 构建方法
    class Image build(vk::Device& device) const;
    std::unique_ptr<Image> buildUnique(vk::Device& device) const;

private:
    std::vector<uint32_t> m_queueFamilyIndices;
};

template <typename ExtensionType>
    requires requires(ExtensionType ext)
    {
        { ext.pNext } -> std::same_as<const void*>;
    }
ImageBuilder& ImageBuilder::setExtension(ExtensionType& extension)
{
    extension.pNext = m_createInfo.pNext;
    m_createInfo.pNext = &extension;
    return *this;
}


class Image : public VulkanAllocated<vk::Image>
{
    using ParentType = VulkanAllocated<vk::Image>;
public:
    Image(vk::Device& device,
          vk::Image handle,
          const vk::Extent3D& extent,
          vk::Format format,
          vk::ImageUsageFlags imageUsage,
          vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1);

    Image(vk::Device& device, const ImageBuilder& builder);

    HUAN_NO_COPY(Image)
    Image(Image&& other) noexcept;
    ~Image() override;

    Image& operator=(Image&&) = delete;

    vk::ImageType getType() const;
    const vk::Extent3D& getExtent() const;
    vk::Format getFormat() const;
    vk::SampleCountFlagBits getSampleCount() const;
    vk::ImageUsageFlags getUsage() const;
    vk::ImageTiling getTiling() const;
    const vk::ImageSubresource& getSubresource() const;
    uint32_t getArrayLayerCount() const;
    std::unordered_set<ImageView*>& getViews();
    vk::DeviceSize getImageRequiredSize() const;

private:
    vk::ImageCreateInfo m_createInfo{};
    vk::ImageSubresource m_subresource{};
    std::unordered_set<ImageView*> m_views{};
};


}