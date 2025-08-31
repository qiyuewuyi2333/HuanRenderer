//
// Created by qiyuewuyi on 8/31/2025.
//

#include "huan/scene_framework/components/mesh.hpp"

namespace huan::framework::scene_graph
{

Mesh::Mesh(const std::string& name) : Component(name)
{
}
Mesh::Mesh(std::string&& name) : Component(name)
{
}
void Mesh::updateBounds(const std::vector<glm::vec3>& vertexData, const std::vector<uint16_t>& indexData)
{
    m_bounds.updateBounds(vertexData, indexData);
}
std::type_index Mesh::getType() const
{
    return typeid(Mesh);
}
const AABB3D& Mesh::getBounds() const
{
    return m_bounds;
}
void Mesh::addSubMesh(SubMesh& subMesh)
{
    m_subMeshes.push_back(&subMesh);
}
const std::vector<SubMesh*>& Mesh::getSubMeshes() const
{
    return m_subMeshes;
}
void Mesh::addNode(Node& node)
{
    m_nodes.push_back(&node);
}
const std::vector<Node*>& Mesh::getNodes() const
{
    return m_nodes;
}
} // namespace huan::framework::scene_graph
