#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;

#define THRESHOLD 1.0

void main() {
    vec3 color = vec3(texture(screenTexture, texCoord));
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));

    color *= step(THRESHOLD, luma);
    FragColor = vec4(color, 1.0);
}
