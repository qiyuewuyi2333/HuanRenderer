//
// Created by 86156 on 5/9/2025.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include "huan/scene_framework/component.hpp"

#include <glm/vec3.hpp>
#include <unordered_map>

namespace huan::framework::scene_graph
{
class Texture;
}

namespace huan::framework::scene_graph
{
/**
 * @brief How the alpha value of the main factor and texture should be interpreted
 */
enum class AlphaMode
{
    Opaque = 0,
    Mask = 1,
    Blend = 2
};

class Material : public Component
{
  public:
    explicit Material(const std::string& name);
    Material(Material&& that) noexcept = default;
    ~Material() override = default;
    [[nodiscard]] std::type_index getType() const override;

    std::unordered_map<std::string, Texture*> m_textures;
    glm::vec3 m_emissive{0.f, 0.f, 0.f};
    bool m_doubleSided{false};
    float m_alphaCutoff{0.5f};
    AlphaMode m_alphaMode{AlphaMode::Opaque};
};
} // namespace huan::framework::scene_graph
#endif // MATERIAL_HPP