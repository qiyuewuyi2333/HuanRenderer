//
// Created by 86156 on 5/9/2025.
//

#ifndef SUB_MESH_HPP
#define SUB_MESH_HPP
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "material.hpp"
#include "huan/backend/vulkan_buffer.hpp"
#include "huan/framework/component.hpp"

namespace huan::engine
{
struct VertexAttribute
{
    vk::Format format = vk::Format::eUndefined;
    uint32_t stride = 0;
    uint32_t offset = 0;
};

class SubMesh : public Component
{
public:
    SubMesh(const std::string& name = {});
    virtual ~SubMesh() = default;
    virtual std::type_index getType() const override;
    vk::IndexType m_indexType{};
    uint32_t m_indexOffset = 0;
    uint32_t m_verticesCount = 0;
    uint32_t m_vertexIndices = 0;
    std::unordered_map<std::string, huan::vulkan::Buffer> m_vertexBuffers;
    Scope<vulkan::Buffer> m_indexBuffer;
    
    void setAttribute(const std::string& name, const VertexAttribute& attribute);
    VertexAttribute getAttribute(const std::string& name) const;

    void setMaterial(const Material& material);
    const Material*  getMaterial() const;
    // TODO: 
    // const Shader
    
};
}
#endif //SUB_MESH_HPP
