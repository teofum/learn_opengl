#version 330 core
in vec3 aPos;
in vec3 aColor;
in vec2 aTexCoord;

out vec3 color;
out vec2 texCoord;

void main() {
    color = aColor;
    texCoord = aTexCoord;
    
    gl_Position = vec4(aPos, 1.0);
}
