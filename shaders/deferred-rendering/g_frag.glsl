#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 texCoord;
in vec3 fragPos;
in mat3 TBN;

struct Material {
    sampler2D diffuse0;
    sampler2D specular0;
    sampler2D normal0;
    bool useNormalMap;
    float shininess;
};

uniform Material material;

void main() {
    gPosition = vec4(fragPos, 1.0);

    vec3 normal = material.useNormalMap ? texture(material.normal0, texCoord).rgb : vec3(0.5, 0.5, 1.0);
    normal = normal * 2.0 - 1.0;
    gNormal = vec4(normalize(TBN * normal), 1.0);

    gAlbedoSpec.rgb = texture(material.diffuse0, texCoord).rgb;
    gAlbedoSpec.a = texture(material.specular0, texCoord).r;
}
