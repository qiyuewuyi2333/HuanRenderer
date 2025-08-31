//
// Created by qiyuewuyi on 8/31/2025.
//

#ifndef HUANRENDERER_AABB3D_HPP
#define HUANRENDERER_AABB3D_HPP
#include "huan/scene_framework/component.hpp"

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <vector>
namespace huan::framework::scene_graph {

/**
 * @brief Axis Aligned Bounding Box
 */
class AABB3D final : public Component {
public:
    AABB3D();

    AABB3D(const glm::vec3& minPoint, const glm::vec3& maxPoint);

    ~AABB3D() override = default;

    [[nodiscard]] std::type_index getType() const override;

    /**
     * @brief Update the bounding box based on the given vertex position
     * @param point The 3D position of a point
     */
    void updateBounds(const glm::vec3& point);

    /**
     * @brief Update the bounding box based on the given submesh vertices
     * @param vertexData The position vertex data
     * @param indexData The index vertex data (optional)
     */
    void updateBounds(const std::vector<glm::vec3>& vertexData,
                      const std::vector<uint16_t>& indexData = {});

    /**
     * @brief Apply a given matrix transformation to the bounding box
     * @param transform The matrix transform to apply
     */
    void transformBounds(glm::mat4& transform);

    /**
     * @brief Scale vector of the bounding box
     * @return vector in 3D space
     */
    [[nodiscard]] glm::vec3 getScale() const;

    /**
     * @brief Center position of the bounding box
     * @return vector in 3D space
     */
    [[nodiscard]] glm::vec3 getCenter() const;

    /**
     * @brief Minimum position of the bounding box
     * @return vector in 3D space
     */
    [[nodiscard]] glm::vec3 getMin() const;

    /**
     * @brief Maximum position of the bounding box
     * @return vector in 3D space
     */
    [[nodiscard]] glm::vec3 getMax() const;

    /**
     * @brief Resets the min and max position coordinates
     */
    void resetBounds();

private:
    glm::vec3 m_min{};
    glm::vec3 m_max{};
};

}  // namespace huan::framework::scene_graph


#endif // HUANRENDERER_AABB3D_HPP
