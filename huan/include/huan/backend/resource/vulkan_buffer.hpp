#pragma once
#include "vulkan_builder_base.hpp"
namespace huan::vulkan
{
class Buffer;
class BufferBuilder : public BuilderBase<BufferBuilder, vk::BufferCreateInfo>
{
  public:
    BufferBuilder(vk::DeviceSize size);
    Buffer build(vk::Device& device) const;
    Scope<Buffer> buildScope(vk::Device& device) const;
    BufferBuilder& setFlags(vk::BufferCreateFlags flags);
    BufferBuilder& setUsage(vk::BufferUsageFlags usage);
};

} // namespace huan::vulkan