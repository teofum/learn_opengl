#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 lightMatrix;
};

layout (std140) uniform DirectionalLightBlock {
    DirectionalLight directionalLight;
};

void main() {
    gl_Position = directionalLight.lightMatrix * model * vec4(aPos, 1.0);
}
