#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;

void main() {
    vec3 color = vec3(texture(screenTexture, texCoord));
    FragColor = vec4(color, 1.0);
}
