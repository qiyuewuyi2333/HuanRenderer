//
// Created by 86156 on 5/7/2025.
//

#ifndef COMPONENT_HPP
#define COMPONENT_HPP
#include <string>
#include <typeindex>


namespace huan::framework
{
class Component
{
public:
    Component() = default;
    Component(const std::string& name);
    Component(Component&& that) noexcept = default;

    virtual ~Component() = default;
    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] virtual std::type_index getType() const = 0;

private:
    std::string m_name;
};

}


#endif //COMPONENT_HPP