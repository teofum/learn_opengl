#version 330 core
in vec3 aPos;
in vec3 aColor;

out vec3 color;

void main() {
    color = aColor;
    gl_Position = vec4(aPos, 1.0);
}
