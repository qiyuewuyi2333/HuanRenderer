//
// Created by 86156 on 5/9/2025.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include "huan/scene_framework/component.hpp"

#include <unordered_map>

namespace huan::framework
{
class Texture;
}
namespace huan::framework
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
    Material(Material&& that) = default;
    ~Material() override = default;
    [[nodiscard]] std::type_index getType() const override;

    std::unordered_map<std::string, Texture*> m_textures;
    
    
};
}
#endif //MATERIAL_HPP
