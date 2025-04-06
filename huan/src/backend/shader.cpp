//
// Created by 86156 on 4/5/2025.
//

#include "huan/backend/shader.hpp"

#include <vulkan/vulkan.hpp>

#include "huan/HelloTriangleApplication.hpp"
#include "huan/log/Log.hpp"

namespace huan
{
    vk::ShaderModule Shader::createShaderModule(const std::vector<uint32_t>& code)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setCodeSize(code.size() * sizeof(uint32_t))
                  .setPCode(code.data());
        
        const vk::ShaderModule shaderModule = HelloTriangleApplication::getInstance()->device.
            createShaderModule(createInfo);
        if (!shaderModule)
        {
            HUAN_CLIENT_BREAK("Failed to create shader module!");
        }
        else
            HUAN_CLIENT_INFO("Created shader module!");
        return shaderModule;
    }
};
