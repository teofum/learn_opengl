#version 330 core
out vec4 FragColor;

in vec3 texCoord;

uniform samplerCube skybox;

void main() {
    vec3 uv = texCoord;
    FragColor = texture(skybox, uv);
}