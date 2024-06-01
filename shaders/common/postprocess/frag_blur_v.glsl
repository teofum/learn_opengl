#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;
uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    vec3 color = vec3(texture(screenTexture, texCoord)) * weight[0];
    for (int i = 1; i < 5; i++) {
        color += vec3(texture(screenTexture, texCoord + vec2(0.0, texelSize.y * i))) * weight[i];
        color += vec3(texture(screenTexture, texCoord - vec2(0.0, texelSize.y * i))) * weight[i];
    }

    FragColor = vec4(color, 1.0);
}
