//
// Created by 86156 on 4/21/2025.
//

#ifndef VULKAN_COMMAND_HPP
#define VULKAN_COMMAND_HPP
#include "huan/common_templates/deferred_system.hpp"

namespace vk
{
class Queue;
class CommandPool;
class CommandBuffer;
class Device;

}

namespace huan
{
/**
 * 奇异模板继承DeferredSystem，Don't repeat yourself.
 */
class CommandSystem final : public DeferredSystem<CommandSystem>
{
public:
    explicit CommandSystem();
    vk::CommandBuffer beginSingleTimeCommands(vk::CommandPool& commandPool);
    void endSingleTimeCommands(vk::CommandPool& commandPool, vk::CommandBuffer& commandBuffer);

private:
    // 系统只会存储非状态信息，
    // NOTE: 即： 系统是无状态的
    vk::Device& deviceHandle;
    vk::Queue& graphicsQueueHandle;
    vk::Queue& transferQueueHandle;

public:
};
}
#endif //VULKAN_COMMAND_HPP