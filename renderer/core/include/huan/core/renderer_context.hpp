#pragma once

#include "huan/core/config.hpp"

struct GLFWwindow;

namespace huan_renderer
{

class RendererContext
{
  public:
    RendererContext() = default;
    virtual ~RendererContext() = default;
    virtual void init() = 0;
    virtual void shutdown() = 0;

    // static Ref<RendererContext> create();
};

} // namespace huan_renderer
