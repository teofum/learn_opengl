#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
layout (std140) uniform LightMatrix {
    mat4 lightMatrix;
};

void main() {
    gl_Position = lightMatrix * model * vec4(aPos, 1.0);
}
