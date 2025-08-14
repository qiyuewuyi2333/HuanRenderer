//
// Created by 86156 on 5/11/2025.
//

#include "huan/backend/shader/spirv_reflection.hpp"

#include "huan/backend/shader.hpp"
#include "huan/backend/resource/resource_system.hpp"

namespace huan::runtime
{
template <vulkan::ShaderResourceType T>
void readShaderResource(const spirv_cross::Compiler& compiler, vk::ShaderStageFlagBits stage,
                        const vulkan::ShaderVariant& variant, std::vector<vulkan::ShaderResource>& shaderResources)
{
    static_assert(false, "Not implemented! Read shader resources of type.");
}

template <spv::Decoration T>
void readResourceDecoration(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource,
                            const vulkan::ShaderVariant& variant, vulkan::ShaderResource& shaderResources)
{
    static_assert(false, "Not implemented! Read resources decoration of type.");
}

template <>
void readResourceDecoration<spv::DecorationLocation>(const spirv_cross::Compiler& compiler,
                                                     const spirv_cross::Resource& resource,
                                                     const vulkan::ShaderVariant& variant,
                                                     vulkan::ShaderResource& shaderResource)
{
    shaderResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
}

template <>
void readResourceDecoration<spv::DecorationDescriptorSet>(const spirv_cross::Compiler& compiler,
                                                          const spirv_cross::Resource& resource,
                                                          const vulkan::ShaderVariant& variant,
                                                          vulkan::ShaderResource& shaderResource)
{
    shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
}

template <>
void readResourceDecoration<spv::DecorationBinding>(const spirv_cross::Compiler& compiler,
                                                    const spirv_cross::Resource& resource,
                                                    const vulkan::ShaderVariant& variant,
                                                    vulkan::ShaderResource& shaderResource)
{
    shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
}

template <>
void readResourceDecoration<spv::DecorationInputAttachmentIndex>(const spirv_cross::Compiler& compiler,
                                                                 const spirv_cross::Resource& resource,
                                                                 const vulkan::ShaderVariant& variant,
                                                                 vulkan::ShaderResource& shaderResource)
{
    shaderResource.inputAttachmentIndex = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
}

template <>
void readResourceDecoration<spv::DecorationNonReadable>(const spirv_cross::Compiler& compiler,
                                                        const spirv_cross::Resource& resource,
                                                        const vulkan::ShaderVariant& variant,
                                                        vulkan::ShaderResource& shaderResource)
{
    shaderResource.qualifiers |= vulkan::ShaderResourceQualifiers::NonReadable;
}

template <>
void readResourceDecoration<spv::DecorationNonWritable>(const spirv_cross::Compiler& compiler,
                                                        const spirv_cross::Resource& resource,
                                                        const vulkan::ShaderVariant& variant,
                                                        vulkan::ShaderResource& shaderResource)
{
    shaderResource.qualifiers |= vulkan::ShaderResourceQualifiers::NonWritable;
}

void readResourceVecSize(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource,
                         const vulkan::ShaderVariant& variant, vulkan::ShaderResource& shaderResource)
{
    const auto& spirvType = compiler.get_type_from_variable(resource.id);
    shaderResource.vecSize = spirvType.vecsize;
    shaderResource.columns = spirvType.columns;
}

void readResourceArraySize(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource,
                           const vulkan::ShaderVariant& variant, vulkan::ShaderResource& shaderResource)
{
    const auto& spirvType = compiler.get_type_from_variable(resource.id);
    shaderResource.arraySize = spirvType.array.empty() ? 1 : spirvType.array[0];
}

void readResourceSize(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource,
                      const vulkan::ShaderVariant& variant, vulkan::ShaderResource& shaderResource)
{
    const auto& spirvType = compiler.get_type_from_variable(resource.id);

    size_t arraySize = 0;
    if (variant.getRuntimeArraySizes().contains(resource.name))
    {
        arraySize = variant.getRuntimeArraySizes().at(resource.name);
    }

    shaderResource.size = compiler.get_declared_struct_size_runtime_array(spirvType, arraySize);
}

void readResourceSize(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRConstant& constant,
                      const vulkan::ShaderVariant& variant, vulkan::ShaderResource& shaderResource)
{
    switch (compiler.get_type(constant.constant_type).basetype)
    {
    case spirv_cross::SPIRType::Boolean:
        shaderResource.size = 1;
        break;
    case spirv_cross::SPIRType::UInt:
    case spirv_cross::SPIRType::Int:
    case spirv_cross::SPIRType::Float:
        shaderResource.size = 4;
        break;
    case spirv_cross::SPIRType::Double:
        shaderResource.size = 8;
        break;
    default:
        shaderResource.size = 0;
        break;
    }
}

template <>
void readShaderResource<vulkan::ShaderResourceType::Input>(const spirv_cross::Compiler& compiler,
                                                           vk::ShaderStageFlagBits stage,
                                                           const vulkan::ShaderVariant& variant,
                                                           std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_shader_resources().stage_inputs)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::Input;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;

        readResourceVecSize(compiler, resource, variant, shaderResource);
        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationLocation>(compiler, resource, variant, shaderResource);

        shaderResources.push_back(shaderResource);
    }
}

template <>
void readShaderResource<vulkan::ShaderResourceType::InputAttachment>(
    const spirv_cross::Compiler& compiler, vk::ShaderStageFlagBits /*stage*/, const vulkan::ShaderVariant& variant,
    std::vector<vulkan::ShaderResource>& shaderResources)
{
    const auto subpassResources = compiler.get_shader_resources().subpass_inputs;
    for (auto& resource : subpassResources)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::InputAttachment;
        shaderResource.stage = vk::ShaderStageFlagBits::eFragment;
        shaderResource.name = resource.name;

        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationInputAttachmentIndex>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, variant, shaderResource);

        shaderResources.push_back(shaderResource);
    }
}

template <>
void readShaderResource<vulkan::ShaderResourceType::Output>(const spirv_cross::Compiler& compiler,
                                                            vk::ShaderStageFlagBits stage,
                                                            const vulkan::ShaderVariant& variant,
                                                            std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_shader_resources().stage_outputs)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::Output;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;

        readResourceVecSize(compiler, resource, variant, shaderResource);
        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationLocation>(compiler, resource, variant, shaderResource);

        shaderResources.push_back(shaderResource);
    }
}

template <>
void readShaderResource<vulkan::ShaderResourceType::Image>(const spirv_cross::Compiler& compiler,
                                                           vk::ShaderStageFlagBits stage,
                                                           const vulkan::ShaderVariant& variant,
                                                           std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_shader_resources().separate_images)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::Image;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;
        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, variant, shaderResource);
        shaderResources.push_back(shaderResource);
    }
}

template <>
void readShaderResource<vulkan::ShaderResourceType::ImageSampler>(const spirv_cross::Compiler& compiler,
                                                                  vk::ShaderStageFlagBits stage,
                                                                  const vulkan::ShaderVariant& variant,
                                                                  std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_shader_resources().separate_samplers)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::ImageSampler;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;
        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, variant, shaderResource);
        shaderResources.push_back(shaderResource);
    }
}

template <>
void readShaderResource<vulkan::ShaderResourceType::ImageStorage>(const spirv_cross::Compiler& compiler,
                                                                  vk::ShaderStageFlagBits stage,
                                                                  const vulkan::ShaderVariant& variant,
                                                                  std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_shader_resources().storage_images)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::ImageStorage;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;
        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, variant, shaderResource);
        shaderResources.push_back(shaderResource);
    }
}

template <>
void readShaderResource<vulkan::ShaderResourceType::BufferUniform>(const spirv_cross::Compiler& compiler,
                                                                   vk::ShaderStageFlagBits stage,
                                                                   const vulkan::ShaderVariant& variant,
                                                                   std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_shader_resources().uniform_buffers)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::BufferUniform;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;
        readResourceSize(compiler, resource, variant, shaderResource);
        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, variant, shaderResource);
        shaderResources.push_back(shaderResource);
    }
}
template <>
void readShaderResource<vulkan::ShaderResourceType::BufferStorage>(const spirv_cross::Compiler& compiler,
                                                                   vk::ShaderStageFlagBits stage,
                                                                   const vulkan::ShaderVariant& variant,
                                                                   std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_shader_resources().storage_buffers)
    {
        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::BufferStorage;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;
        readResourceSize(compiler, resource, variant, shaderResource);
        readResourceArraySize(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationNonReadable>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationNonWritable>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationDescriptorSet>(compiler, resource, variant, shaderResource);
        readResourceDecoration<spv::DecorationBinding>(compiler, resource, variant, shaderResource);
        shaderResources.push_back(shaderResource);
    }
}

bool SPIRVReflection::reflectShaderResources(vk::ShaderStageFlagBits stage, const std::vector<uint32_t>& spirvCode,
                                             const vulkan::ShaderVariant& shaderVariant,
                                             std::vector<vulkan::ShaderResource>& shaderResources)
{
    spirv_cross::CompilerGLSL compiler{spirvCode};

    auto options = compiler.get_common_options();
    options.enable_420pack_extension = true;
    compiler.set_common_options(options);

    parseShaderResources(compiler, stage, shaderVariant, shaderResources);
    parsePushConstants(compiler, stage, shaderVariant, shaderResources);
    parseSpecializationConstants(compiler, stage, shaderVariant, shaderResources);

    return true;
}

void SPIRVReflection::parseShaderResources(const spirv_cross::Compiler& compiler, vk::ShaderStageFlagBits stage,
                                           const vulkan::ShaderVariant& variant,
                                           std::vector<vulkan::ShaderResource>& shaderResources)
{
    readShaderResource<vulkan::ShaderResourceType::Input>(compiler, stage, variant, shaderResources);
    readShaderResource<vulkan::ShaderResourceType::InputAttachment>(compiler, stage, variant, shaderResources);

    readShaderResource<vulkan::ShaderResourceType::Output>(compiler, stage, variant, shaderResources);

    readShaderResource<vulkan::ShaderResourceType::Image>(compiler, stage, variant, shaderResources);
    readShaderResource<vulkan::ShaderResourceType::ImageSampler>(compiler, stage, variant, shaderResources);
    readShaderResource<vulkan::ShaderResourceType::ImageStorage>(compiler, stage, variant, shaderResources);

    readShaderResource<vulkan::ShaderResourceType::BufferUniform>(compiler, stage, variant, shaderResources);
    readShaderResource<vulkan::ShaderResourceType::BufferStorage>(compiler, stage, variant, shaderResources);
}

void SPIRVReflection::parsePushConstants(const spirv_cross::Compiler& compiler, vk::ShaderStageFlagBits stage,
                                         const vulkan::ShaderVariant& variant,
                                         std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (auto& resource : compiler.get_shader_resources().push_constant_buffers)
    {
        const auto& spirvType = compiler.get_type_from_variable(resource.id);

        uint32_t offset = std::numeric_limits<uint32_t>::max();
        for (auto i = 0; i < spirvType.member_types.size(); ++i)
        {
            auto memberOffset = compiler.get_member_decoration(resource.base_type_id, i, spv::DecorationOffset);
            offset = std::min(offset, memberOffset);
        }

        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::PushConstant;
        shaderResource.stage = stage;
        shaderResource.name = resource.name;
        shaderResource.offset = offset;
        readResourceSize(compiler, resource, variant, shaderResource);
        shaderResource.size -= shaderResource.offset;

        shaderResources.push_back(shaderResource);
    }
}

void SPIRVReflection::parseSpecializationConstants(const spirv_cross::Compiler& compiler, vk::ShaderStageFlagBits stage,
                                                   const vulkan::ShaderVariant& variant,
                                                   std::vector<vulkan::ShaderResource>& shaderResources)
{
    for (const auto& resource : compiler.get_specialization_constants())
    {
        auto& spirvValue = compiler.get_constant(resource.id);

        vulkan::ShaderResource shaderResource{};
        shaderResource.type = vulkan::ShaderResourceType::SpecializationConstant;
        shaderResource.stage = stage;
        shaderResource.name = compiler.get_name(resource.id);
        shaderResource.offset = 0;
        shaderResource.constantID = resource.constant_id;
        readResourceSize(compiler, spirvValue, variant, shaderResource);

        shaderResources.push_back(shaderResource);
    }
}
} // namespace huan::engine