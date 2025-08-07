//
// Created by 86156 on 5/12/2025.
//

#pragma once
#include "huan/scene_framework/component.hpp"

namespace huan::vulkan
{
class Sampler;
class Image;
} // namespace huan::vulkan

namespace huan::framework
{
class Texture : public Component
{
  public:
    explicit Texture(const std::string& name);
    explicit Texture(std::string&& name);
    Texture(Texture&& that) = default;
    virtual ~Texture() = default;
    [[nodiscard]] virtual std::type_index getType() const override;
    void setImage(vulkan::Image* image);
    [[nodiscard]] vulkan::Image* getImage() const;
    void setSampler(vulkan::Sampler sampler);

private:
    vulkan::Image* 
};

} // namespace huan::framework