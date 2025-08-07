//
// Created by 86156 on 5/7/2025.
//
#pragma once
#include <string>
#include <typeindex>

namespace huan::framework::scene_graph
{
class Component
{
public:
    Component() = default;
    explicit Component(std::string&& name);
    explicit Component(const std::string& name);
    Component(Component&& that) noexcept = default;

    virtual ~Component() = default;
    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] virtual std::type_index getType() const = 0;

private:
    std::string m_name;
};

} // namespace huan::framework