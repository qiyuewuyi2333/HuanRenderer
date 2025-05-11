//
// Created by 86156 on 5/9/2025.
//

#include "../../../include/huan/framework/components/sub_mesh.hpp"

namespace huan::engine
{
SubMesh::SubMesh(const std::string& name)
    : Component(name)
{
}

std::type_index SubMesh::getType() const
{
    return typeid(SubMesh);
}

void SubMesh::setAttribute(const std::string& name, const VertexAttribute& attribute)
{
    m_vertexAttributes[name] = attribute;
    computeShaderVariant();
}

std::optional<VertexAttribute> SubMesh::getAttribute(const std::string& name) const
{
    auto it = m_vertexAttributes.find(name);
    if (it != m_vertexAttributes.end() )
    {
        return it->second;
    }
    return {};
}

void SubMesh::setMaterial(const Material& material)
{
    m_material = &material;
    computeShaderVariant();
}

const Material* SubMesh::getMaterial() const
{
    return m_material;
}

const vulkan::ShaderVariant& SubMesh::getShaderVariant() const
{
    return m_shaderVariant;
}

vulkan::ShaderVariant& SubMesh::getMutableShaderVariant()
{
    return m_shaderVariant;
}

void SubMesh::computeShaderVariant()
{
    m_shaderVariant.clear();
    if (m_material)
    {
        // TODO: Texture
        for ()
    }
}
}