#pragma once

#include <string>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace huan_renderer
{
class VulkanPipeline
{
  public:
    VulkanPipeline() = default;
    ~VulkanPipeline() = default;

    void init();
    void cleanup();

  private:
    VkShaderModule create_shader_module(const std::vector<char>& code);
    static std::vector<char> read_file(const std::string& filename);

  private:
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;
};
}; // namespace huan_renderer
