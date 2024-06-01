#version 330 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D screenTexture;

void main() {
    vec3 hdr = vec3(texture(screenTexture, texCoord));
    vec3 color = hdr / (hdr + vec3(1.0));
    FragColor = vec4(color, 1.0);
}
