//
// Created by TU on 2025/8/11.
//
#pragma once
#include "vulkan_resource.hpp"


namespace huan::runtime::vulkan
{
class Image;

class ImageView : public VulkanResource<vk::ImageView>
{
    using ParentType = VulkanResource<vk::ImageView>;

public:
    ImageView(const Image& image,
              vk::ImageViewType viewType,
              vk::Format format = vk::Format::eUndefined,
              uint32_t baseMipLevel = 0,
              uint32_t baseArrayLayer = 0,
              uint32_t nMipLevels = 0,
              uint32_t nArrayLayers = 0);
    HUAN_NO_COPY(ImageView)
    ImageView(ImageView&& that) noexcept;
    ~ImageView() override;
    ImageView& operator=(ImageView&& that) = delete;

    vk::Format getFormat() const;
    const Image& getImage() const;
    void setImage(Image& image);
    vk::ImageSubresourceLayers getSubresourceLayers() const;
    vk::ImageSubresourceRange getSubresourceRange() const;

private:
    Image* m_image = nullptr;
    vk::Format m_format;
    vk::ImageSubresourceRange m_subresourceRange;

};

}