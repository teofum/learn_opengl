#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;
uniform int screenWidth;
uniform int screenHeight;

void main() {
    float xOffset = 1.0 / screenWidth;
    float yOffset = 1.0 / screenHeight;

    vec2 offsets[9] = vec2[](
        vec2(-xOffset, yOffset),
        vec2(0.0, yOffset),
        vec2(xOffset, yOffset),
        vec2(-xOffset, 0.0),
        vec2(0.0, 0.0),
        vec2(xOffset, 0.0),
        vec2(-xOffset, -yOffset),
        vec2(0.0, -yOffset),
        vec2(xOffset, -yOffset)
    );

    float kernel[9] = float[](
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1
    );

    vec3 color = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        color += kernel[i] * vec3(texture(screenTexture, texCoord + offsets[i]));
    }

    FragColor = vec4(color, 1.0);
}
