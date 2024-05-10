#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 texCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float ratio;

void main() {
    vec4 sampled = mix(texture(texture1, texCoord), texture(texture2, texCoord), ratio);
    FragColor = sampled;
}