//#version 450
//
//layout(location = 0) out vec4 outColor;
//
//void main() {
//    // Tiffany Blue
//    vec3 tiffanyBlue = vec3(129.0/255.0, 216.0/255.0, 207.0/255.0);
//    outColor = vec4(tiffanyBlue, 1.0);
//}

#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}