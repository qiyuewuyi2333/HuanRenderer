//
// Created by 86156 on 5/7/2025.
//

#ifndef SCENE_HPP
#define SCENE_HPP
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <typeindex>

#include "huan/common.hpp"

namespace huan::framework::scene_graph
{
class Component;
class Node;

/**
 * Root collection for a bunch of nodes
 */
class Scene
{
  public:
    Scene() = default;
    Scene(const std::string& name);
    Scene(Scene&& that) noexcept = default;
    Scene(const Scene& that) = delete;
    Scene& operator=(Scene&& that) = default;
    Scene& operator=(const Scene& that) = delete;
    virtual ~Scene() = default;

    void setName(const std::string& name);
    [[nodiscard]] const std::string& getName() const;

    void setNodes(std::vector<Node*> nodes);
    void addNode(Scope<Node>&& node);
    void addChild(Node* node);

    void addComponent(Scope<Component>&& component);
    void addComponent(Scope<Component>&& component, Node* node);
    void setComponents(const std::type_index& typeInfo, std::vector<Scope<Component>>&& components);
    template <typename T>
    void setComponents(std::vector<Scope<T>>&& components);
    template <typename T>
    void clearComponents();
    template <typename T>
    std::vector<T*> getComponents() const;
    [[nodiscard]] const std::vector<Scope<Component>>& getComponents(const std::type_index& type_info) const;

    template <class T>
    [[nodiscard]] bool hasComponent() const;
    [[nodiscard]] bool hasComponent(const std::type_index& type_info) const;

    [[nodiscard]] Node* findNode(const std::string& name) const;
    void setRootNode(Node* root);
    [[nodiscard]] Node* getRootNode() const;

  private:
    std::string m_name;
    std::vector<Scope<Node>> m_nodes;
    Node* m_root = nullptr;
    std::unordered_map<std::type_index, std::vector<Scope<Component>>> m_components;
};

template <typename T>
void Scene::setComponents(std::vector<Scope<T>>&& components)
{
    std::vector<Scope<Component>> result(components.size());
    std::transform(components.begin(), components.end(), result.begin(),
                   [](Scope<T>& component) -> Scope<Component> { return Scope<Component>(std::move(component)); });
    setComponents(typeid(T), std::move(result));
}

template <typename T>
void Scene::clearComponents()
{
    setComponents(typeid(T), std::vector<Scope<Component>>{});
}

template <typename T>
std::vector<T*> Scene::getComponents() const
{
    std::vector<T*> result;
    if (hasComponent(typeid(T)))
    {
        auto&& scene_components = getComponents(typeid(T));

        result.resize(scene_components.size());
        std::transform(scene_components.begin(), scene_components.end(), result.begin(),
                       [](const Scope<Component>& component) -> T* { return dynamic_cast<T*>(component.get()); });
    }
    return result;
}

template <class T>
bool Scene::hasComponent() const
{
    return hasComponent(typeid(T));
}

} // namespace huan::framework

#endif // SCENE_HPP