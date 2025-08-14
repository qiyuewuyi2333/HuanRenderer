//
// Created by 86156 on 5/12/2025.
//

#pragma once
#include "huan/scene_framework/component.hpp"

namespace huan::runtime::vulkan
{
class Sampler;
class Image;
} // namespace huan::vulkan

namespace huan::framework::scene_graph
{
class Texture : public Component
{
  public:
    explicit Texture(const std::string& name);
    explicit Texture(std::string&& name);
    Texture(Texture&& that) = default;
    virtual ~Texture() = default;
    [[nodiscard]] virtual std::type_index getType() const override;
    void setImage(runtime::vulkan::Image* image);
    [[nodiscard]] runtime::vulkan::Image* getImage() const;
    void setSampler(runtime::vulkan::Sampler sampler);

private:
};

} // namespace huan::framework