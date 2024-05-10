#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 texCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float ratio;

void main() {
    vec2 coord2 = vec2(texCoord.x, 1.0 - texCoord.y);
    vec4 sampled = mix(texture(texture1, texCoord), texture(texture2, coord2), ratio);
    FragColor = sampled;
}