//
// Created by 86156 on 4/5/2025.
//

#ifndef SHADER_HPP
#define SHADER_HPP

#include <vector>
#include <string>

namespace vk
{
    class ShaderModule;
}

namespace huan
{
    class Shader
    {
    public:
        static vk::ShaderModule createShaderModule(const std::vector<uint32_t>& code);
    };
    
}
#endif //SHADER_HPP
