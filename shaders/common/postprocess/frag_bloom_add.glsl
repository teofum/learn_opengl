#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;
uniform sampler2D bloomTexture;

#define BLOOM_STRENGTH 0.5

void main() {
    vec3 color = vec3(texture(screenTexture, texCoord));
    color += vec3(texture(bloomTexture, texCoord)) * BLOOM_STRENGTH;

    FragColor = vec4(color, 1.0);
}
