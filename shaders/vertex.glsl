#version 330 core
in vec3 aPos;
in vec3 aColor;
in vec2 aTexCoord;

out vec3 color;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    color = aColor;
    texCoord = aTexCoord;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
