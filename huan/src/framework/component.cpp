//
// Created by 86156 on 5/7/2025.
//

#include "huan/framework/component.hpp"


namespace huan::engine
{

Component::Component(const std::string& name)
    :m_name(name)
{
}

const std::string& Component::getName() const
{
    return m_name;
}
}

