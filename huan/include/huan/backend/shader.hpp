//
// Created by 86156 on 4/5/2025.
//

#ifndef SHADER_HPP
#define SHADER_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include <vulkan/vulkan.hpp>

namespace huan::runtime::vulkan
{
enum class ShaderResourceType
{
    Input,
    InputAttachment,
    Output,
    Image,
    ImageSampler,
    ImageStorage,
    Sampler,
    BufferUniform,
    BufferStorage,
    PushConstant,
    SpecializationConstant,
    All
};

enum class ShaderResourceMode
{
    Static,
    Dynamic,
    UpdateAfterBind,
};

struct ShaderResourceQualifiers
{
    enum : uint32_t
    {
        None = 0,
        NonReadable = 1 << 0,
        NonWritable = 1 << 1,
    };
};

/**
 * Resource description used in shader
 */
struct ShaderResource final
{
    vk::ShaderStageFlags stage;
    ShaderResourceType type;
    ShaderResourceMode mode;
    uint32_t set;
    uint32_t binding;
    uint32_t location;
    uint32_t inputAttachmentIndex;
    uint32_t vecSize;   // vecSize = 1 for scalar, 2 for vec2, 3 for vec3, 4 for vec4
    uint32_t columns;   // columns = 1 for scalar, 2 for mat2, 3 for mat3, 4 for mat4
    uint32_t arraySize; // arraySize = 0 for not array, 1 for array, 2 for array of array, ...
    uint32_t offset;
    uint32_t size;
    uint32_t constantID;
    uint32_t qualifiers;
    std::string name;
};

class ShaderSource final
{
  public:
    ShaderSource() = default;

    explicit ShaderSource(const std::string& fileName);

    [[nodiscard]] size_t getID() const;

    [[nodiscard]] std::string_view getFileName() const;

    void setSource(std::string_view source);

    [[nodiscard]] std::string_view getSource() const;

  private:
    uint32_t m_id{};

    std::string m_fileName;

    std::string m_source;
};

class ShaderVariant final
{
  public:
    ShaderVariant() = default;
    ShaderVariant(std::string&& preamble, std::vector<std::string>&& processes);
    [[nodiscard]] uint32_t getID() const;

    void addDefs(const std::vector<std::string>& definitions);
    void addDef(const std::string& definition);
    void addUnDef(const std::string& undefinition);

    void addRuntimeArraySize(const std::string& name, size_t size);
    void setRuntimeArraySizes(const std::unordered_map<std::string, size_t>& runtimeArraySizes);
    void setRuntimeArraySize(const std::string& name, size_t size);

    [[nodiscard]] const std::string& getPreamble() const;
    [[nodiscard]] const std::vector<std::string>& getProcesses() const;

    [[nodiscard]] const std::unordered_map<std::string, size_t>& getRuntimeArraySizes() const;

    void clear();

  private:
    void updateID();

  private:
    uint32_t m_id{};
    std::string m_preamble;
    std::vector<std::string> m_processes;
    std::unordered_map<std::string, size_t> m_runtimeArraySizes;
};

class ShaderModule
{
  public:
    ShaderModule(vk::Device& device, vk::ShaderStageFlagBits stage, const ShaderSource& glslSource,
                 const std::string& entryPoint, const ShaderVariant& variant);
    ShaderModule(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&& that) noexcept;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule& operator=(ShaderModule&& that) = delete;

    [[nodiscard]] size_t getID() const;
    [[nodiscard]] vk::ShaderStageFlagBits getStage() const;
    [[nodiscard]] const std::string& getEntryPoint() const;
    [[nodiscard]] const std::vector<ShaderResource>& getResources() const;
    [[nodiscard]] const std::string& getInfoLog() const;
    [[nodiscard]] const std::vector<uint32_t>& getBinary() const;
    [[nodiscard]] const std::string& getDebugName() const;

    void setResourceMode(const std::string& resourceName, const ShaderResourceMode& resourceMode);

  private:
    vk::Device& deviceHandle;
    uint32_t m_id{};
    vk::ShaderStageFlagBits m_stage{};
    std::string m_entryPoint{};
    std::string m_debugName{};
    std::string m_infoLog{};
    std::vector<ShaderResource> m_resources{};
    std::vector<uint32_t> m_binary{}; // spir-v code
};

} // namespace huan::engine::vulkan
#endif // SHADER_HPP