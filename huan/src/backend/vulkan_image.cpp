//
// Created by 86156 on 4/20/2025.
//

// #include <huan/backend/vulkan_image.hpp>
//
// #include "huan/HelloTriangleApplication.hpp"
// #include "huan/backend/vulkan_buffer.hpp"
// #include "huan/log/Log.hpp"
//
// namespace huan::vulkan
// {
// Image::Image()
// {
//     this->m_image = nullptr;
//     this->m_imageView = nullptr;
//     this->m_allocation = nullptr;
//     this->m_writeType = WriteType::Dynamic;
//     this->m_extent = vk::Extent3D{0, 0, 0};
//
//     this->m_viewInfo = vk::ImageViewCreateInfo{};
// }
//
// Image::Image(Image& that)
// {
//     if (this != &that)
//     {
//         std::swap(this->m_image, that.m_image);
//         std::swap(this->m_imageView, that.m_imageView);
//         std::swap(this->m_allocation, that.m_allocation);
//         std::swap(this->m_writeType, that.m_writeType);
//         std::swap(this->m_extent, that.m_extent);
//         
//         std::swap(this->m_viewInfo, that.m_viewInfo);
//     }
// }
//
// // void VulkanImageSystem::updateData(VulkanImage& image, void* srcData, VkDeviceSize size, VkDeviceSize offset)
// // {
// //     static auto& deviceHandle = HelloTriangleApplication::getInstance()->device;
// //
// //     if (!image.m_image)
// //         HUAN_CORE_BREAK(
// //         "VulkanImage::updateData: m_image is nullptr, can't write in, please invoke create function first. ");
// //
// //     if (image.m_writeType == VulkanImage::WriteType::Dynamic)
// //     {
// //         // Map it
// //         if (image.m_data == nullptr)
// //             image.m_data = deviceHandle.mapMemory(image.m_memory, offset, size);
// //
// //         if (srcData)
// //             memcpy((char*)image.m_data + offset, (char*)srcData + offset, size);
// //     }
// //     else if (image.m_writeType == VulkanImage::WriteType::Static)
// //     {
// //         // Just copy it, we should create a staging buffer
// //         auto stagingBuffer = VulkanBuffer::create(, size, vk::ImageUsageFlagBits::eTransferDst,
// //                                                  vk::MemoryPropertyFlagBits::eHostVisible |
// //                                                  vk::MemoryPropertyFlagBits::eHostCoherent);
// //         stagingBuffer->updateData(srcData, size, offset);
// //         copyFrom(stagingBuffer->m_buffer, size);
// //         stagingBuffer.reset();
// //     }
// // }
// }