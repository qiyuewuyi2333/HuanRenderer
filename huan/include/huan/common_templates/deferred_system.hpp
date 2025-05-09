//
// Created by 86156 on 4/21/2025.
//

#ifndef DEFERRED_SYSTEM_HPP
#define DEFERRED_SYSTEM_HPP
namespace huan
{
template <class T>
class DeferredSystem
{
public:
    inline static T* getInstance()
    {
        if (s_instance == nullptr)
        {
            s_instance = new T();
        }
        return s_instance;
    }
    
protected:
    inline static T* s_instance = nullptr;
};
}

#endif //DEFERRED_SYSTEM_HPP
