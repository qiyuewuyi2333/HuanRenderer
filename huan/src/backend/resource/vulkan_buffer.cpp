#include "huan/backend/resource/vulkan_buffer.hpp"

namespace huan::vulkan
{

BufferBuilder::BufferBuilder(vk::DeviceSize size)
    : ParentType(vk::BufferCreateInfo{{}, size})
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


Buffer::Buffer(vk::Device& device, const BufferBuilder& builder)
    : ParentType(builder.getAllocationCreateInfo(), device)
{
    this->setHandle(this->createBuffer(builder.getCreateInfo()));
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
    return this->getDeviceHandle().getBufferAddress({.buffer = getHandle()});
}

}