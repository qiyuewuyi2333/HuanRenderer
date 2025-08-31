//
// Created by qiyuewuyi on 8/31/2025.
//

#ifndef HUANRENDERER_MESH_HPP
#define HUANRENDERER_MESH_HPP
#include "AABB3D.hpp"
#include "huan/scene_framework/component.hpp"
#include "sub_mesh.hpp"

#include <glm/vec3.hpp>
#include <vector>

namespace huan::framework::scene_graph
{

class Mesh final : public Component
{
  public:
    Mesh(const std::string& name);
    Mesh(std::string&& name);
    ~Mesh() override = default;
    void updateBounds(const std::vector<glm::vec3>& vertexData, const std::vector<uint16_t>& indexData = {});
    virtual std::type_index getType() const override;
    const AABB3D& getBounds() const;
    void addSubMesh(SubMesh& subMesh);
    const std::vector<SubMesh*>& getSubMeshes() const;
    void addNode(Node& node);
    const std::vector<Node*>& getNodes() const;

  private:
    AABB3D m_bounds;
    std::vector<SubMesh*> m_subMeshes;
    std::vector<Node*> m_nodes;
};
} // namespace huan::framework::scene_graph

#endif // HUANRENDERER_MESH_HPP
