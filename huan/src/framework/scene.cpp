//
// Created by 86156 on 5/7/2025.
//

#include "huan/common.hpp"
#include "huan/log/Log.hpp"
#include "huan/scene_framework/scene.hpp"

#include <queue>

#include "huan/scene_framework/component.hpp"
#include "huan/scene_framework/node.hpp"

namespace huan::framework::scene_graph
{

Scene::Scene(const std::string& name)
    : m_name(name)
{
}

void Scene::setName(const std::string& name)
{
    m_name = name;
}

const std::string& Scene::getName() const
{
    return m_name;
}

void Scene::setNodes(std::vector<Node*> nodes)
{
    HUAN_CORE_ASSERT(nodes.empty(), "The nodes in this scene has been set. ")
    nodes = std::move(nodes);
}

void Scene::addNode(Scope<Node>&& node)
{
    m_nodes.emplace_back(std::move(node));
}

void Scene::addChild(Node* node)
{
    m_root->addChild(node);
}

void Scene::addComponent(Scope<Component>&& component)
{
    if (component)
    {
        m_components[component->getType()].emplace_back(std::move(component));
    }
}

void Scene::addComponent(Scope<Component>&& component, Node* node)
{
    if (component)
    {
        node->setComponent(component.get());
        m_components[component->getType()].emplace_back(std::move(component));
    }
}

void Scene::setComponents(const std::type_index& typeInfo, std::vector<Scope<Component>>&& components)
{
    m_components[typeInfo] = std::move(components);
}

const std::vector<Scope<Component>>& Scene::getComponents(const std::type_index& type_info) const
{
    return m_components.at(type_info);
}

bool Scene::hasComponent(const std::type_index& type_info) const
{
    auto it = m_components.find(type_info);
    return (it != m_components.end() && !it->second.empty());
}

/**
 * 根据名字进行层次优先遍历查找第一个Node
 */
Node* Scene::findNode(const std::string& name) const
{
    for (auto& node : m_root->getChildren())
    {
        // 层次优先遍历
        std::queue<Node*> traverseNodes{};
        traverseNodes.push(m_root);
        while (!traverseNodes.empty())
        {
            auto node = traverseNodes.front();
            traverseNodes.pop();
            if (node->getName() == name)
            {
                return node;
            }
            for (auto& child : node->getChildren())
            {
                traverseNodes.push(child);
            }
        }
    }
    return nullptr;
}

void Scene::setRootNode(Node* root)
{
    m_root = root;
}

Node* Scene::getRootNode() const
{
    return m_root;
}
}