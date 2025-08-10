//
// Created by 86156 on 4/5/2025.
//

#include "huan/backend/shader.hpp"

#include <ranges>
#include <vulkan/vulkan.hpp>

#include "huan/HelloTriangleApplication.hpp"
#include "huan/backend/shader/glsl_compiler.hpp"
#include "huan/backend/shader/spirv_reflection.hpp"
#include "huan/log/Log.hpp"
#include "huan/utils/file_system.hpp"

namespace huan::runtime::vulkan
{
// vk::ShaderModule Shader::createShaderModule(const std::vector<uint32_t>& code)
// {
//     vk::ShaderModuleCreateInfo createInfo;
//     createInfo.setCodeSize(code.size() * sizeof(uint32_t))
//               .setPCode(code.data());
//     
//     const vk::ShaderModule shaderModule = HelloTriangleApplication::getInstance()->device.
//         createShaderModule(createInfo);
//     if (!shaderModule)
//     {
//         HUAN_CLIENT_BREAK("Failed to create shader module!");
//     }
//     else
//         HUAN_CLIENT_INFO("Created shader module!");
//     return shaderModule;
// }
std::string preprocessShader(std::string_view source)
{
    std::string finalSource;
    finalSource.reserve(source.size() * 2 + 10);

    auto lines = source | std::views::split('\n');
    for (auto line : lines)
    {
        std::string_view lineView{line.begin(), line.end()};
        if (lineView.starts_with("#include \""))
        {
            std::string_view includePath = lineView.substr(10);
            size_t lastQuote = includePath.find('"');
            if (!includePath.empty() && lastQuote != std::string_view::npos)
            {
                includePath = includePath.substr(0, lastQuote);
            }
            finalSource.append(
                preprocessShader(FileSystem::loadFile(includePath)));
        }
        else
        {
            finalSource.append(lineView);
        }
    }
    return finalSource;
}

ShaderSource::ShaderSource(const std::string& fileName)
    : m_fileName(fileName), m_source(FileSystem::loadFile(fileName))
{
    std::hash<std::string> hash;
    m_id = hash(m_source);
}

size_t ShaderSource::getID() const
{
    return m_id;
}

std::string_view ShaderSource::getFileName() const
{
    return m_fileName;
}

void ShaderSource::setSource(std::string_view source)
{
    m_source = source;
    constexpr std::hash<std::string> hash;
    m_id = hash(m_source);
}

std::string_view ShaderSource::getSource() const
{
    return m_source;
}

ShaderVariant::ShaderVariant(std::string&& preamble, std::vector<std::string>&& processes)
    : m_preamble(std::move(preamble)), m_processes(std::move(processes))
{
    updateID();
}

uint32_t ShaderVariant::getID() const
{
    return m_id;
}

void ShaderVariant::addDefs(const std::vector<std::string>& definitions)
{
    for (auto& def : definitions)
    {
        addDef(def);
    }
}

void ShaderVariant::addDef(const std::string& definition)
{
    m_processes.push_back("D" + definition);
    std::string temp = definition;
    size_t posEqual = temp.find_first_of('=');
    if (posEqual != std::string_view::npos)
    {
        temp[posEqual] = ' ';
    }
    m_preamble.append("#define " + temp + "\n");

    updateID();
}

void ShaderVariant::addUnDef(const std::string& undefinition)
{
    m_processes.push_back("U" + undefinition);
    m_preamble.append("#undef " + undefinition + "\n");
    updateID();
}

void ShaderVariant::addRuntimeArraySize(const std::string& name, size_t size)
{
    m_runtimeArraySizes[name] = size;
}

void ShaderVariant::setRuntimeArraySizes(const std::unordered_map<std::string, size_t>& runtimeArraySizes)
{
    m_runtimeArraySizes = runtimeArraySizes;
}

void ShaderVariant::setRuntimeArraySize(const std::string& name, size_t size)
{
    m_runtimeArraySizes[name] = size;
}

const std::string& ShaderVariant::getPreamble() const
{
    return m_preamble;
}

const std::vector<std::string>& ShaderVariant::getProcesses() const
{
    return m_processes;
}

const std::unordered_map<std::string, size_t>& ShaderVariant::getRuntimeArraySizes() const
{
    return m_runtimeArraySizes;
}

void ShaderVariant::clear()
{
    m_preamble.clear();
    m_processes.clear();
    m_runtimeArraySizes.clear();
    updateID();
}

void ShaderVariant::updateID()
{
    std::hash<std::string> hash;
    m_id = hash(m_preamble);
}

ShaderModule::ShaderModule(vk::Device& device, vk::ShaderStageFlagBits stage, const ShaderSource& glslSource,
                           const std::string& entryPoint, const ShaderVariant& variant)
    : deviceHandle(HelloTriangleApplication::getInstance()->device),
      m_stage(stage), m_entryPoint(entryPoint)
{
    m_debugName = fmt::format("[Source]: {}, [Variant]: {:X}, [EntryPoint]: {}", glslSource.getFileName(),
                              variant.getID(), entryPoint);
    if (entryPoint.empty())
    {
        HUAN_CORE_BREAK("Renderer: Shader Init Failed!")
    }
    const auto source = glslSource.getSource();
    if (source.empty())
    {
        HUAN_CORE_BREAK("Renderer: Shader Init Failed!")
    }

    const auto glslFinalSource = preprocessShader(source);
    GLSLCompiler compiler;
    if (!compiler.compileToSPIRV(stage, glslFinalSource, entryPoint, variant, m_binary, m_infoLog))
    {
        HUAN_CORE_ERROR("Renderer: Shader compilation failed for shader [{}]", glslSource.getFileName())
        HUAN_CORE_BREAK(m_infoLog)
    }
    SPIRVReflection spirvReflection;
    if (!SPIRVReflection::reflectShaderResources(stage, m_binary, variant, m_resources))
    {
        HUAN_CORE_BREAK("Renderer: Shader Init Failed!")
    }
    std::hash<std::string> hash{};
    m_id = hash(std::string(m_binary.begin(), m_binary.end()));
}

ShaderModule::ShaderModule(ShaderModule&& that) noexcept
    : deviceHandle(that.deviceHandle),
      m_id(that.m_id),
      m_stage(that.m_stage),
      m_entryPoint(that.m_entryPoint),
      m_debugName(that.m_debugName),
      m_infoLog(std::move(that.m_infoLog)),
      m_resources(std::move(that.m_resources)),
      m_binary(std::move(that.m_binary))
{
    that.m_stage = {};
}

size_t ShaderModule::getID() const
{
    return m_id;
}

vk::ShaderStageFlagBits ShaderModule::getStage() const
{
    return m_stage;
}

const std::string& ShaderModule::getEntryPoint() const
{
    return m_entryPoint;
}

const std::vector<ShaderResource>& ShaderModule::getResources() const
{
    return m_resources;
}

const std::string& ShaderModule::getInfoLog() const
{
    return m_infoLog;
}

const std::vector<uint32_t>& ShaderModule::getBinary() const
{
    return m_binary;
}

const std::string& ShaderModule::getDebugName() const
{
    return m_debugName;
}

void ShaderModule::setResourceMode(const std::string& resourceName, const ShaderResourceMode& resourceMode)
{
    auto it = std::ranges::find_if(m_resources, [&resourceName](const ShaderResource& resource) {
        return resource.name == resourceName;
    });

    if (it != m_resources.end())
    {
        if (resourceMode == ShaderResourceMode::Dynamic)
        {
            if (it->type == ShaderResourceType::BufferUniform ||
                it->type == ShaderResourceType::BufferStorage)
            {
                it->mode = resourceMode;
            }
            else
            {
                HUAN_CORE_BREAK("Renderer: Resource[{}] does not support dynamic. ", resourceName)
            }
        }
        else if (resourceMode == ShaderResourceMode::Static)
        {
            it->mode = resourceMode;
        }
        else if (resourceMode == ShaderResourceMode::UpdateAfterBind)
        {
            if (it->type == ShaderResourceType::Image ||
                it->type == ShaderResourceType::ImageSampler ||
                it->type == ShaderResourceType::ImageStorage)
            {
                it->mode = resourceMode;
            }
            else
            {
                HUAN_CORE_BREAK("Renderer: Resource[{}] does not support update after bind. ", resourceName)
            }
        }
        else
        {
            HUAN_CORE_BREAK("Renderer: Resource[{}] does not support mode[{}].", resourceName,
                            static_cast<int>(resourceMode))
        }
    }
    else
    {
        HUAN_CORE_BREAK("Renderer: Resource[{}] does not exist.", resourceName)
    }
}

};