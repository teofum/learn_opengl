#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 fragPos;
in mat3 TBN;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse0;
    sampler2D specular0;
    sampler2D normal0;
    bool useNormalMap;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    mat4 lightMatrices[6];
};

uniform Material material;
layout (std140) uniform PointLightBlock0 {
    PointLight pointLight0;
};
layout (std140) uniform PointLightBlock1 {
    PointLight pointLight1;
};
uniform samplerCube shadowMap0;
uniform samplerCube shadowMap1;
uniform float farPlane;

#define SAMPLES 20

vec3 sampleOffsetDirections[SAMPLES] = vec3[](
    vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
    vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float calculateShadow(PointLight light, samplerCube shadowMap) {
    vec3 fragToLight = fragPos - light.position;
    float fragDepth = length(fragToLight);
    float viewDistance = length(viewPos - fragPos);

    float bias = 0.05;
    float diskRadius = (1.0 + viewDistance / farPlane) / 25.0;
    float shadow = 0.0;

    for (int i = 0; i < SAMPLES; i++) {
        float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).x;
        closestDepth *= farPlane;
        shadow += step(fragDepth - bias, closestDepth);
    }
    shadow /= float(SAMPLES);

    return shadow;
}
vec3 calculatePointLight(PointLight light, vec3 diffMap, vec3 specMap, vec3 viewDir, samplerCube shadowMap) {
    vec3 ambient = diffMap * light.ambient;

    vec3 normal = material.useNormalMap ? texture(material.normal0, texCoord).rgb : vec3(0.5, 0.5, 1.0);
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    float dist = length(light.position - fragPos);
    float attenuation = light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * dist * dist;

    float shadow = calculateShadow(light, shadowMap);

    return (ambient + (diffuse + specular) * shadow) / attenuation;
}

void main() {
    vec3 diffMap = vec3(texture(material.diffuse0, texCoord));
    vec3 specMap = vec3(texture(material.specular0, texCoord));
    vec3 viewDir = normalize(fragPos - viewPos);

    vec3 color = vec3(0.0);
    color += calculatePointLight(pointLight0, diffMap, specMap, viewDir, shadowMap0);
    color += calculatePointLight(pointLight1, diffMap, specMap, viewDir, shadowMap1);
    FragColor = vec4(color, 1.0);
}