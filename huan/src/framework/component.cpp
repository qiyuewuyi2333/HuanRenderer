//
// Created by 86156 on 5/7/2025.
//

#include <utility>

#include "huan/scene_framework/component.hpp"

namespace huan::framework::scene_graph
{

Component::Component(std::string&& name) : m_name(std::move(name))
{
}
Component::Component(const std::string& name)
    :m_name(name)
{
    
}

const std::string& Component::getName() const
{
    return m_name;
}
} // namespace huan::framework
