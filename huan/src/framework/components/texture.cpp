//
// Created by 86156 on 5/12/2025.
//

#include "huan/scene_framework/components/texture.hpp"

namespace huan::framework::scene_graph {

Texture::Texture(const std::string& name)
    :Component(name)
{
}
Texture::Texture(std::string&& name)
    :Component(name)
{
}

std::type_index Texture::getType() const
{
    return typeid(Texture);
}
} // namespace huan::framework
