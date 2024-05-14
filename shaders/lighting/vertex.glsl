#version 330 core
in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoord;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    texCoord = aTexCoord;
    mat3 normalModel = transpose(inverse(mat3(model)));
    normal = normalize(normalModel * aNormal);
    vec4 modelPos = model * vec4(aPos, 1.0);
    fragPos = vec3(modelPos);

    gl_Position = projection * view * modelPos;
}
