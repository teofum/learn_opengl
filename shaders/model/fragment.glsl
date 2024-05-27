#version 330 core
#define N_POINT_LIGHTS 4
out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse0;
    sampler2D specular0;
    float shininess;
};

struct PointLight {
    mat4 lightMatrix;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

uniform Material material;
layout (std140) uniform PointLightBlock {
    PointLight pointLight;
};

vec3 calculatePointLight(PointLight light, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    float dist = length(light.position - fragPos);
    float attenuation = light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * dist * dist;

    return (ambient + diffuse + specular) / attenuation;
}

void main() {
    vec3 diffMap = vec3(texture(material.diffuse0, texCoord));
    vec3 specMap = vec3(texture(material.specular0, texCoord));
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 color = vec3(0.0);
    color += calculatePointLight(pointLight, diffMap, specMap, viewDir);
    FragColor = vec4(color, 1.0);
}