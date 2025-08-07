#pragma once
#include "vulkan_allocated.hpp"
#include "vulkan_builder_base.hpp"

namespace huan::engine::run_time::vulkan
{
class Buffer;

class BufferBuilder : public BuilderBase<BufferBuilder, vk::BufferCreateInfo>
{
    using ParentType = BuilderBase<BufferBuilder, vk::BufferCreateInfo>;

public:
    explicit BufferBuilder(vk::DeviceSize size);
    [nodiscard] Buffer build(vk::Device& device) const;
    Scope<Buffer> buildScope(vk::Device& device) const;
    BufferBuilder& setFlags(vk::BufferCreateFlags flags);
    BufferBuilder& setUsage(vk::BufferUsageFlags usage);
};

class Buffer final : public VulkanAllocated<vk::Buffer>
{
    friend class BufferBuilder;

    using ParentType = VulkanAllocated<vk::Buffer>;

public:
    explicit Buffer(vk::Device& device, const BufferBuilder& builder);
    HUAN_NO_COPY(Buffer)
    Buffer(Buffer&& that) noexcept = default;
    Buffer& operator=(Buffer&& that) noexcept = default;
    ~Buffer() override;

    vk::DeviceSize getSize() const;
    vk::DeviceSize getDeviceAddress() const;

private:
    vk::DeviceSize m_size;
};

} // namespace huan::vulkan