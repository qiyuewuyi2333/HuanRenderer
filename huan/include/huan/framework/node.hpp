//
// Created by 86156 on 5/7/2025.
//

#ifndef NODE_HPP
#define NODE_HPP
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace huan::engine
{
class Component;

class Node
{
public:
    Node(size_t id, const std::string& name);
    virtual ~Node() = default;
    [[nodiscard]] size_t getId() const;
    [[nodiscard]] std::string getName() const;
    
    void setParent(Node* parent);
    [[nodiscard]] Node* getParent() const;
    void addChild(Node* child);
    [[nodiscard]] const std::vector<Node*>& getChildren() const;
    
    void setComponent(Component* component);
    template<typename T>
    T* getComponent() const;
    [[nodiscard]] Component* getComponent(std::type_index type) const;
    template<typename T>
    bool hasComponent();
    [[nodiscard]] bool hasComponent(std::type_index type) const;
    
private:
    const size_t m_id;
    std::string m_name;
    Node* m_parent = nullptr;
    std::vector<Node*> m_children;
    std::unordered_map<std::type_index, Component*> m_components;
};

template <typename T>
T* Node::getComponent() const
{
    return dynamic_cast<T*>(getComponent(typeid(T)));
}

template <typename T>
bool Node::hasComponent()
{
    return hasComponent(typeid(T));
}

}

#endif //NODE_HPP