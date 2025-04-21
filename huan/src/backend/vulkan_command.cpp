//
// Created by 86156 on 4/21/2025.
//
#include <huan/backend/vulkan_command.hpp>
#include "huan/HelloTriangleApplication.hpp"

namespace huan
{

CommandSystem::CommandSystem(): deviceHandle(HelloTriangleApplication::getInstance()->device),
                                graphicsQueueHandle(HelloTriangleApplication::getInstance()->graphicsQueue),
                                transferQueueHandle(HelloTriangleApplication::getInstance()->transferQueue)
{
}

vk::CommandBuffer CommandSystem::beginSingleTimeCommands(vk::CommandPool& commandPool)
{
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandPool(commandPool)
                .setCommandBufferCount(1);

    vk::CommandBuffer commandBuffer = deviceHandle.allocateCommandBuffers(allocateInfo)[0];

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void CommandSystem::endSingleTimeCommands(vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer)
{
    commandBuffer.end();
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer);

    auto& submitQueue = graphicsQueueHandle;

    submitQueue.submit(submitInfo, nullptr);
    submitQueue.waitIdle();

    deviceHandle.freeCommandBuffers(commandPool, 1, &commandBuffer);
}


}