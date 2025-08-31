//
// Created by qiyuewuyi on 8/31/2025.
//

#include "huan/scene_framework/components/AABB3D.hpp"

#include <glm/common.hpp>
#include <glm/glm.hpp>

namespace huan::framework::scene_graph
{

AABB3D::AABB3D() {
    resetBounds();
}

AABB3D::AABB3D(const glm::vec3& minPoint, const glm::vec3& maxPoint)
    : m_min{minPoint},
      m_max{maxPoint} {
}

std::type_index AABB3D::getType() const {
    return typeid(AABB3D);
}

void AABB3D::updateBounds(const glm::vec3& point) {
    m_min = glm::min(m_min, point);
    m_max = glm::max(m_max, point);
}

void AABB3D::updateBounds(const std::vector<glm::vec3>& vertexData,
                          const std::vector<uint16_t>& indexData) {
    if (!indexData.empty()) {
        for (uint16_t index : indexData) {
            updateBounds(vertexData[index]);
        }
    } else {
        for (const auto& vertex : vertexData) {
            updateBounds(vertex);
        }
    }
}

void AABB3D::transformBounds(glm::mat4& transform) {
    // 初始化为第一个角点
    glm::vec4 transformed = glm::vec4(m_min, 1.0f) * transform;
    m_min = m_max = glm::vec3(transformed);

    // 处理剩余7个角点
    auto updateCorner = [this, &transform](float x, float y, float z) {
        glm::vec3 corner = glm::vec3(glm::vec4(x, y, z, 1.0f) * transform);
        m_min = glm::min(m_min, corner);
        m_max = glm::max(m_max, corner);
    };

    updateCorner(m_min.x, m_min.y, m_max.z);
    updateCorner(m_min.x, m_max.y, m_min.z);
    updateCorner(m_min.x, m_max.y, m_max.z);
    updateCorner(m_max.x, m_min.y, m_min.z);
    updateCorner(m_max.x, m_min.y, m_max.z);
    updateCorner(m_max.x, m_max.y, m_min.z);
    updateCorner(m_max.x, m_max.y, m_max.z);
}

glm::vec3 AABB3D::getScale() const {
    return m_max - m_min;
}

glm::vec3 AABB3D::getCenter() const {
    return (m_min + m_max) * 0.5f;
}

glm::vec3 AABB3D::getMin() const {
    return m_min;
}

glm::vec3 AABB3D::getMax() const {
    return m_max;
}

void AABB3D::resetBounds() {
    m_min = std::numeric_limits<glm::vec3>::max();
    m_max = std::numeric_limits<glm::vec3>::min();
}
}
