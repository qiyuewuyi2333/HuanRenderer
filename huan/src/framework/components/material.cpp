//
// Created by 86156 on 5/9/2025.
//

#include "huan/scene_framework/components/material.hpp"
namespace huan::framework
{
Material::Material(const std::string& name) : Component(name)
{
}
std::type_index Material::getType() const
{
    return typeid(Material);
}

} // namespace huan::framework