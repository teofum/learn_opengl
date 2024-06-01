#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;

void main() {
    vec3 color = vec3(texture(screenTexture, texCoord));
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    FragColor = vec4(gray, gray, gray, 1.0);
}
