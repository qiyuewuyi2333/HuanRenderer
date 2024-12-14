#pragma once
#include <memory>

// define export and import macro
#ifdef _WIN32
#ifdef HUAN_EXPORT
#define HUAN_API __declspec(dllexport)
#else
#define HUAN_API __declspec(dllimport)
#endif
#else
#define HUAN_API
#endif

// Macro to ban the copy constructor in class
#define HUAN_NO_COPY(Class)                                                                                            \
    Class(const Class&) = delete;                                                                                      \
    Class& operator=(const Class&) = delete;

// Macro to ban the move constructor in class
#define HUAN_NO_MOVE(Class)                                                                                            \
    Class(Class&&) = delete;                                                                                           \
    Class& operator=(Class&&) = delete;

// Alias name to define Ref and Scope
template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr Ref<T> create_ref(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using Scope = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Scope<T> create_scope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
