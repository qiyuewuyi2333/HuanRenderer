//
// Created by 86156 on 5/9/2025.
//

#ifndef SUB_MESH_HPP
#define SUB_MESH_HPP
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "material.hpp"
#include "huan/common.hpp"
#include "huan/backend/shader.hpp"
#include "huan/scene_framework/component.hpp"
#include "huan/scene_framework/node.hpp"

#include <optional>


namespace huan::runtime::vulkan
{
class Buffer;
}

namespace huan::framework::scene_graph
{
struct VertexAttribute
{
    vk::Format format = vk::Format::eUndefined;
    uint32_t stride = 0;
    uint32_t offset = 0;
};

class SubMesh final : public Component
{
public:
    explicit SubMesh(const std::string& name = {});
    ~SubMesh() override = default;
    std::type_index getType() const override;
    vk::IndexType m_indexType{};
    uint32_t m_indexOffset = 0;
    uint32_t m_verticesCount = 0;
    uint32_t m_vertexIndices = 0;
    std::unordered_map<std::string, runtime::vulkan::Buffer> m_vertexBuffers;
    Scope<runtime::vulkan::Buffer> m_indexBuffer;

    void setAttribute(const std::string& name, const VertexAttribute& attribute);
    std::optional<VertexAttribute> getAttribute(const std::string& name) const;

    void setMaterial(const Material& material);
    const Material* getMaterial() const;

    const runtime::vulkan::ShaderVariant& getShaderVariant() const;
    runtime::vulkan::ShaderVariant& getMutableShaderVariant();

private:
    std::unordered_map<std::string, VertexAttribute> m_vertexAttributes;
    const Material* m_material = nullptr;
    runtime::vulkan::ShaderVariant m_shaderVariant;

    void computeShaderVariant();
};
}
#endif //SUB_MESH_HPP