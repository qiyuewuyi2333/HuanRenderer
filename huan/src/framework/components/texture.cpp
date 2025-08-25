//
// Created by 86156 on 5/12/2025.
//

#include "huan/scene_framework/components/texture.hpp"
#include "huan/backend/resource/vulkan_image.hpp"
#include "huan/backend/resource/sampler.hpp"

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

void Texture::setImage(runtime::vulkan::Image* image)
{
    m_image = createScope<runtime::vulkan::Image>(std::move(*image));
}

runtime::vulkan::Image* Texture::getImage() const
{
    return m_image.get();
}

void Texture::setSampler(runtime::vulkan::Sampler sampler)
{
    m_sampler = createScope<runtime::vulkan::Sampler>(std::move(sampler));
}
} // namespace huan::framework
