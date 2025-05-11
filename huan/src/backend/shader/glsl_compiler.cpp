//
// Created by 86156 on 5/11/2025.
//

#include <vulkan/vulkan.hpp>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <spirv/GlslangToSpv.h>

#include "huan/backend/shader/glsl_compiler.hpp"

#include "StandAlone/DirStackFileIncluder.h"

namespace huan::engine
{


EShLanguage FindShaderLanguage(vk::ShaderStageFlagBits stage)
{
    switch (stage)
    {
    case vk::ShaderStageFlagBits::eVertex:
        return EShLangVertex;
    case vk::ShaderStageFlagBits::eTessellationControl:
        return EShLangTessControl;
    case vk::ShaderStageFlagBits::eTessellationEvaluation:
        return EShLangTessEvaluation;
    case vk::ShaderStageFlagBits::eGeometry:
        return EShLangGeometry;
    case vk::ShaderStageFlagBits::eFragment:
        return EShLangFragment;
    case vk::ShaderStageFlagBits::eCompute:
        return EShLangCompute;
    case vk::ShaderStageFlagBits::eRaygenKHR:
        return EShLangRayGen;
    case vk::ShaderStageFlagBits::eAnyHitKHR:
        return EShLangAnyHit;
    case vk::ShaderStageFlagBits::eClosestHitKHR:
        return EShLangClosestHit;
    case vk::ShaderStageFlagBits::eMissKHR:
        return EShLangMiss;
    case vk::ShaderStageFlagBits::eIntersectionKHR:
        return EShLangIntersect;
    case vk::ShaderStageFlagBits::eCallableKHR:
        return EShLangCallable;
    case vk::ShaderStageFlagBits::eMeshEXT:
        return EShLangMesh;
    case vk::ShaderStageFlagBits::eTaskEXT:
        return EShLangTask;

    default:
        // 可选：添加断言或日志提示非法输入
        return EShLangVertex;
    }
}

void GLSLCompiler::setTargetEnv(glslang::EShTargetLanguage targetLanguage,
                                glslang::EShTargetLanguageVersion targetLanguageVersion)
{
    envTargetLanguage = targetLanguage;
    envTargetLanguageVersion = targetLanguageVersion;
}

void GLSLCompiler::resetTargetEnv()
{
    envTargetLanguage = glslang::EShTargetLanguage::EShTargetNone;
    envTargetLanguageVersion = glslang::EShTargetLanguageVersion::EShTargetSpv_1_0;
}

bool GLSLCompiler::compileToSPIRV(vk::ShaderStageFlagBits stage,
                                  const std::string& glslSource,
                                  std::string_view entryPoint,
                                  const vulkan::ShaderVariant& shaderVariant,
                                  std::vector<uint32_t>& spirvCode,
                                  std::string& infoLog)
{
    glslang::InitializeProcess();
    auto messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);
    EShLanguage language = FindShaderLanguage(stage);
    std::string_view source = glslSource;

    const char* fileNameList[1] = {""};
    const char* shaderSource = source.data();

    glslang::TShader shader(language);
    shader.setStringsWithLengthsAndNames(&shaderSource, nullptr, fileNameList, 1);
    shader.setEntryPoint(entryPoint.data());
    shader.setSourceEntryPoint(entryPoint.data());
    shader.setPreamble(shaderVariant.getPreamble().data());
    shader.addProcesses(shaderVariant.getProcesses());
    if (GLSLCompiler::envTargetLanguage != glslang::EShTargetNone)
    {
        shader.setEnvTarget(GLSLCompiler::envTargetLanguage, GLSLCompiler::envTargetLanguageVersion);
    }

    DirStackFileIncluder includeDir;
    includeDir.pushExternalLocalDirectory("shaders");

    if (!shader.parse(GetDefaultResources(), 450, false, messages, includeDir))
    {
        infoLog = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
        return false;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages))
    {
        infoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        return false;
    }

    if (shader.getInfoLog())
        infoLog += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
    if (program.getInfoLog())
        infoLog += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog()) + "\n";

    glslang::TIntermediate *intermediate = program.getIntermediate(language);

    if (!intermediate)
    {
        infoLog += "Failed to get intermediate\n";
        return false;
    }

    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*intermediate, spirvCode, &logger);

    infoLog += logger.getAllMessages() + "\n";
    glslang::FinalizeProcess();
    return true;
}
}