#include "huan/platform/vulkan/vulkan_pipeline.hpp"
#include "huan/platform/vulkan/vulkan_context.hpp"
#include <fstream>
#include <iostream>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
void VulkanPipeline::init()
{
    auto vert_shader_code = read_file("shaders/vert.spv");
    auto frag_shader_code = read_file("shaders/frag.spv");

    VkShaderModule vertex_shader_module = create_shader_module(vert_shader_code);
    VkShaderModule fragment_shader_module = create_shader_module(frag_shader_code);
    VkPipelineShaderStageCreateInfo vert_stage_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                                    .module = vertex_shader_module,
                                                    .pName = "main",
                                                    .pSpecializationInfo = nullptr};
    VkPipelineShaderStageCreateInfo frag_stage_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                    .module = fragment_shader_module,
                                                    .pName = "main"};

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};
    VkPipelineVertexInputStateCreateInfo vertex_input_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .vertexAttributeDescriptionCount = 0};
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};
    VkPipelineViewportStateCreateInfo viewport_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr, // Dynamic state
        .scissorCount = 1,
        .pScissors = nullptr // Dynamic state
    };

    VkPipelineRasterizationStateCreateInfo rasterizer_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f,
        .depthBiasEnable = VK_FALSE};

    VkPipelineMultisampleStateCreateInfo multisampling_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE};

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE};

    VkPipelineColorBlendStateCreateInfo color_blending_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment};
    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()};

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 0, .pushConstantRangeCount = 0};

    if (vkCreatePipelineLayout(VulkanContext::get_instance().get_vk_device().m_device, &pipeline_layout_info, nullptr,
                               &m_pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                  .stageCount = 2,
                                                  .pStages = shader_stages,
                                                  .pVertexInputState = &vertex_input_info,
                                                  .pInputAssemblyState = &input_assembly_info,
                                                  .pViewportState = &viewport_state_info,
                                                  .pRasterizationState = &rasterizer_info,
                                                  .pMultisampleState = &multisampling_info,
                                                  .pColorBlendState = &color_blending_info,
                                                  .pDynamicState = &dynamic_state_info,
                                                  .layout = m_pipeline_layout,
                                                  .renderPass =
                                                      VulkanContext::get_instance().get_vk_render_pass().m_render_pass,
                                                  .subpass = 0,
                                                  .basePipelineHandle = VK_NULL_HANDLE,
                                                  .basePipelineIndex = -1};

    if (vkCreateGraphicsPipelines(VulkanContext::get_instance().get_vk_device().m_device, VK_NULL_HANDLE, 1,
                                  &pipeline_info, nullptr, &m_graphics_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(VulkanContext::get_instance().get_vk_device().m_device, vertex_shader_module, nullptr);
    vkDestroyShaderModule(VulkanContext::get_instance().get_vk_device().m_device, fragment_shader_module, nullptr);
}
void VulkanPipeline::cleanup()
{
    if (m_graphics_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(VulkanContext::get_instance().get_vk_device().m_device, m_graphics_pipeline, nullptr);
        m_graphics_pipeline = VK_NULL_HANDLE;
    }

    if (m_pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(VulkanContext::get_instance().get_vk_device().m_device, m_pipeline_layout, nullptr);
        m_pipeline_layout = VK_NULL_HANDLE;
    }
}
VkShaderModule VulkanPipeline::create_shader_module(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                         .codeSize = code.size(),
                                         .pCode = reinterpret_cast<const uint32_t*>(code.data())};
    VkShaderModule shader_module;
    if (vkCreateShaderModule(VulkanContext::get_instance().get_vk_device().m_device, &create_info, nullptr,
                             &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shader_module;
}

std::vector<char> VulkanPipeline::read_file(const std::string& filename)
{
    /*
     * ate : Start reading at the end of the file
     *    So we can use the pos to get the size of file to determine the size of vec
     * binary : Read the file as binary file(avoid text transformations)
     */
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    // Back to the head of file
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return std::move(buffer);
}
}; // namespace huan_renderer
