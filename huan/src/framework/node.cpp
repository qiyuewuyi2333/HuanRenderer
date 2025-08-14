//
// Created by 86156 on 5/7/2025.
//

#include "huan/scene_framework/node.hpp"

#include "huan/scene_framework/component.hpp"

namespace huan::framework::scene_graph
{

Node::Node(size_t id, const std::string& name)
    :m_id(id), m_name(name)
{
}

size_t Node::getId() const
{
    return m_id;
}

std::string Node::getName() const
{
    return m_name;
}

void Node::setParent(Node* parent)
{
    m_parent = parent;
}

Node* Node::getParent() const
{
    return m_parent;
}

void Node::addChild(Node* child)
{
    m_children.push_back(child);
}

const std::vector<Node*>& Node::getChildren() const
{
    return m_children;
}

void Node::setComponent(Component* component)
{
    auto it = m_components.find(component->getType());
    if (it == m_components.end())
    {
        m_components.insert(std::make_pair(component->getType(), component));
    }
    else if (it->second != component)
    {
        it->second = component;
    }
}

Component* Node::getComponent(const std::type_index type) const
{
    return m_components.at(type);
}

bool Node::hasComponent(const std::type_index type) const
{
    return m_components.contains(type);
}


}
