#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec2 texCoord;
out vec3 fragPos;
out mat3 TBN;

uniform mat4 model;
layout (std140) uniform Matrices {
    mat4 view;
    mat4 projection;
};

void main() {
    texCoord = aTexCoord;

    mat3 normalModel = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalModel * aNormal);
    vec3 T = normalize(normalModel * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    vec4 modelPos = model * vec4(aPos, 1.0);
    fragPos = vec3(modelPos);

    gl_Position = projection * view * modelPos;
}
