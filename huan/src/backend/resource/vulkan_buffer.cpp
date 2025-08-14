#include "huan/backend/resource/vulkan_buffer.hpp"

namespace huan::runtime::vulkan
{
#pragma region BufferBuilder
BufferBuilder::BufferBuilder(VmaAllocator allocator, vk::DeviceSize size)
    : ParentType(allocator, vk::BufferCreateInfo{{}, size})
{
}

Buffer BufferBuilder::build(vk::Device& device) const
{
    return Buffer(device, *this);
}

Scope<Buffer> BufferBuilder::buildScope(vk::Device& device) const
{
    return createScope<Buffer>(device, *this);
}

BufferBuilder& BufferBuilder::setFlags(vk::BufferCreateFlags flags)
{
    m_createInfo.flags = flags;
    return *this;
}

BufferBuilder& BufferBuilder::setUsage(vk::BufferUsageFlags usage)
{
    m_createInfo.usage = usage;
    return *this;
}
#pragma endregion

#pragma region Buffer
Buffer::Buffer(vk::Device& device, const BufferBuilder& builder)
    : ParentType(device, builder.getAllocator(), builder.getAllocationCreateInfo())
{
    this->setHandle(this->createBuffer(builder.getCreateInfo()));
    this->m_size = builder.getCreateInfo().size;
#ifdef HUAN_DEBUG
    if (!builder.getDebugName().empty())
    {
        this->setDebugName(builder.getDebugName());
    }
#endif
}

Buffer::~Buffer()
{
    this->destroyBuffer(getHandle());
}

vk::DeviceSize Buffer::getSize() const
{
    return m_size;
}

vk::DeviceSize Buffer::getDeviceAddress() const
{
    return this->getDeviceHandle().getBufferAddress(vk::BufferDeviceAddressInfo{ getHandle()});
}

#pragma endregion
}