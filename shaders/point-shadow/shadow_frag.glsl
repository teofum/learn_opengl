#version 330 core

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    mat4 lightMatrices[6];
};

layout (std140) uniform PointLightBlock {
    PointLight pointLight;
};
uniform float farPlane;

in vec4 fragPos;

void main() {
    float lightDistance = length(fragPos.xyz - pointLight.position);
    gl_FragDepth = lightDistance / farPlane;
}
