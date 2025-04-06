#pragma once
#include "huan/common.hpp"
#include <spdlog/spdlog.h>

namespace huan
{
class HUAN_API Log
{
  public:
    static void init();
    static Ref<spdlog::logger>& getCoreLogger()
    {
        return sCoreLogger;
    }
    static Ref<spdlog::logger>& getClientLogger()
    {
        return sClientLogger;
    }

  private:
    static Ref<spdlog::logger> sCoreLogger;
    static Ref<spdlog::logger> sClientLogger;
};
} // namespace huan

#ifdef HUAN_ENABLE_LOG
#define HUAN_CORE_TRACE(...) ::huan::Log::getCoreLogger()->trace(__VA_ARGS__);
#define HUAN_CORE_INFO(...) ::huan::Log::getCoreLogger()->info(__VA_ARGS__);
#define HUAN_CORE_WARN(...) ::huan::Log::getCoreLogger()->warn(__VA_ARGS__);
#define HUAN_CORE_ERROR(...) ::huan::Log::getCoreLogger()->error(__VA_ARGS__);
#define HUAN_CORE_CRITICAL(...) ::huan::Log::getCoreLogger()->critical(__VA_ARGS__);

#define HUAN_CLIENT_TRACE(...) ::huan::Log::getClientLogger()->trace(__VA_ARGS__);
#define HUAN_CLIENT_INFO(...) ::huan::Log::getClientLogger()->info(__VA_ARGS__);
#define HUAN_CLIENT_WARN(...) ::huan::Log::getClientLogger()->warn(__VA_ARGS__);
#define HUAN_CLIENT_ERROR(...) ::huan::Log::getClientLogger()->error(__VA_ARGS__);
#define HUAN_CLIENT_CRITICAL(...) ::huan::Log::getClientLogger()->critical(__VA_ARGS__);
#else
#define HUAN_CORE_TRACE(...)
#define HUAN_CORE_INFO(...)
#define HUAN_CORE_WARN(...)
#define HUAN_CORE_ERROR(...)
#define HUAN_CORE_CRITICAL(...)
#define HUAN_CLIENT_TRACE(...)
#define HUAN_CLIENT_INFO(...)
#define HUAN_CLIENT_WARN(...)
#define HUAN_CLIENT_ERROR(...)
#define HUAN_CLIENT_CRITICAL(...)
#endif
