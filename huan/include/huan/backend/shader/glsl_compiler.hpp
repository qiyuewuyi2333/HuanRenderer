//
// Created by 86156 on 5/11/2025.
//

#pragma once
#include <glslang/Public/ShaderLang.h>

#include "huan/backend/shader.hpp"

namespace huan::runtime
{
/**
 * Convert SPIR-V code from GLSL source
 */
class GLSLCompiler final
{
public:
    static void setTargetEnv(glslang::EShTargetLanguage targetLanguage,
                             glslang::EShTargetLanguageVersion targetLanguageVersion);

    // Reset ENV to default
    static void resetTargetEnv();

    bool compileToSPIRV(vk::ShaderStageFlagBits stage,
                        const std::string& glslSource,
                        std::string_view entryPoint,
                        const vulkan::ShaderVariant& shaderVariant,
                        std::vector<uint32_t>& spirvCode,
                        std::string& infoLog
        );

private:
    inline static glslang::EShTargetLanguage envTargetLanguage = glslang::EShTargetLanguage::EShTargetNone;
    inline static glslang::EShTargetLanguageVersion envTargetLanguageVersion = glslang::EShTargetLanguageVersion::EShTargetSpv_1_0;
};
}