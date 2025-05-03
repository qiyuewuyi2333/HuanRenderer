#version 450 

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// Define a uniform buffer object struct for uniform buffer
// NOTE: 而且使用binding指令 其实和 layout(location = 0) 是一样的
// A set is a collection of bindings. But one binding can also represent an array of resources.
layout(set = 0, binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0);
    fragColor = color;
    fragTexCoord = texCoord;
}
