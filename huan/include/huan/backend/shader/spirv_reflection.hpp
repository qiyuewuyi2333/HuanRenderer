//
// Created by 86156 on 5/11/2025.
//

#pragma once
#include <vector>

#include "spirv_cross/spirv_glsl.hpp"

namespace vk
{
enum class ShaderStageFlagBits : unsigned int;
}

namespace huan::runtime
{

namespace vulkan
{
struct ShaderResource;
class ShaderVariant;
}

/**
 * 生成一系列的ShaderResource 基于 SPIRV反射代码 和 ShaderVariant
 */
class SPIRVReflection final
{
public:
    static bool reflectShaderResources(vk::ShaderStageFlagBits stage,
                                       const std::vector<uint32_t>& spirvCode,
                                       const vulkan::ShaderVariant& shaderVariant,
                                       std::vector<vulkan::ShaderResource>& shaderResources
        );

private:
    static void parseShaderResources(const spirv_cross::Compiler& compiler,
                              vk::ShaderStageFlagBits stage,
                              const vulkan::ShaderVariant& variant,
                              std::vector<vulkan::ShaderResource>& shaderResources);
    static void parsePushConstants(const spirv_cross::Compiler& compiler,
                            vk::ShaderStageFlagBits stage,
                            const vulkan::ShaderVariant& variant,
                            std::vector<vulkan::ShaderResource>& shaderResources);
    static void parseSpecializationConstants(const spirv_cross::Compiler& compiler,
                                      vk::ShaderStageFlagBits stage,
                                      const vulkan::ShaderVariant& variant,
                                      std::vector<vulkan::ShaderResource>& shaderResources);
};
}