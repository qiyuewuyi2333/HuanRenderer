//
// Created by 86156 on 4/4/2025.
//

#ifndef COMMON_HPP
#define COMMON_HPP
#include <memory>

#if defined(HUAN_BUILD_SHARED)
#define HUAN_API __declspec(dllexport)
#elif defined(HUAN_BUILD_STATIC)
#define HUAN_API
#else
#define HUAN_API __declspec(dllimport)
#endif

#define HUAN_NO_COPY(classname)                                                                                        \
    classname(const classname&) = delete;                                                                              \
    classname& operator=(const classname&) = delete;
#define HUAN_NO_MOVE(classname)                                                                                        \
    classname(classname&&) = delete;                                                                                   \
    classname& operator=(classname&&) = delete;

#if defined(HUAN_INNER_VISIBLE)
#define INNER_VISIBLE public
#define INNER_PROTECT public
#else
#define INNER_VISIBLE private
#define INNER_PROTECT protected
#endif

#ifdef HUAN_DEBUG
#define HUAN_ENABLE_ASSERTS
#endif

#ifdef HUAN_ENABLE_ASSERT
#define HUAN_CLIENT_BREAK(...)                                                                                         \
    {                                                                                                                  \
        HUAN_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__)                                                        \
        assert(false);                                                                                                 \
    }
#define HUAN_CORE_BREAK(...)                                                                                           \
    {                                                                                                                  \
        HUAN_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__)                                                        \
        assert(false);                                                                                                 \
    }
#define HUAN_CLIENT_ASSERT(x, ...)                                                                                     \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
            HUAN_CLIENT_BREAK(__VA_ARGS__)                                                                             \
    }
#define HUAN_CORE_ASSERT(x, ...)                                                                                       \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
            HUAN_CORE_BREAK(__VA_ARGS__)                                                                               \
    }
#define VK_CHECK(x)                                                                                                    \
    auto res = x;                                                                                                      \
    if (res != VK_SUCCESS)                                                                                             \
    {                                                                                                                  \
        HUAN_CORE_BREAK("Vulkan Error: {0}, {1}", #x, (int)res);                                                            \
    }
#else
#define HUAN_CLIENT_ASSERT(x, ...)
#define HUAN_CORE_ASSERT(x, ...)
#endif

#define BIND_EVENT_FUNC(x) std::bind(&x, this, std::placeholders::_1)

namespace huan
{
template <typename T>
using Scope = ::std::unique_ptr<T>;

template <typename T, typename... Args>
Scope<T> createScope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = ::std::shared_ptr<T>;

template <typename T, typename... Args>
Ref<T> createRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using Weak = ::std::weak_ptr<T>;

using RendererID = unsigned int;
} // namespace huan
#endif // COMMON_HPP