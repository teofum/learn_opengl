#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;
out vec4 fragPosLightSpace;

uniform mat4 model;
layout (std140) uniform Matrices {
    mat4 view;
    mat4 projection;
};
layout (std140) uniform LightMatrix {
    mat4 lightMatrix;
};

void main() {
    texCoord = aTexCoord;
    mat3 normalModel = transpose(inverse(mat3(model)));
    normal = normalize(normalModel * aNormal);
    vec4 modelPos = model * vec4(aPos, 1.0);
    fragPos = vec3(modelPos);
    fragPosLightSpace = lightMatrix * modelPos;

    gl_Position = projection * view * modelPos;
}
