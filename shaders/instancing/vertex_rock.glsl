#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 model;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

layout (std140) uniform Matrices {
    mat4 view;
    mat4 projection;
};

void main() {
    texCoord = aTexCoord;
    mat3 normalModel = transpose(inverse(mat3(model)));
    normal = normalize(normalModel * aNormal);
    vec4 modelPos = model * vec4(aPos, 1.0);
    fragPos = vec3(modelPos);

    gl_Position = projection * view * modelPos;
}
