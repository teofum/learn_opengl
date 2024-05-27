#version 330 core
#define N_POINT_LIGHTS 4
out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 lightMatrix;
};

struct PointLight {
    mat4 lightMatrix;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

struct SpotLight {
    mat4 lightMatrix;
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    vec2 angles;
};

uniform Material material;
layout (std140) uniform DirectionalLightBlock {
    DirectionalLight directionalLight;
};
layout (std140) uniform PointLightBlock0 {
    PointLight pointLight0;
};
layout (std140) uniform PointLightBlock1 {
    PointLight pointLight1;
};
layout (std140) uniform PointLightBlock2 {
    PointLight pointLight2;
};
layout (std140) uniform PointLightBlock3 {
    PointLight pointLight3;
};
layout (std140) uniform SpotLightBlock {
    SpotLight spotLight;
};

vec3 calculateDirectionalLight(DirectionalLight light, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    float diff = max(dot(-light.direction, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(light.direction, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    return ambient + diffuse + specular;
}

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

vec3 calculateSpotLight(SpotLight light, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, -light.direction);
    float cutoff = smoothstep(light.angles.x, light.angles.y, theta);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    float dist = length(light.position - fragPos);
    float attenuation = light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * dist * dist;

    return (ambient + (diffuse + specular) * cutoff) / attenuation;
}

void main() {
    vec3 diffMap = vec3(texture(material.diffuse, texCoord));
    vec3 specMap = vec3(texture(material.specular, texCoord));
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 color = vec3(0.0);
    color += calculateDirectionalLight(directionalLight, diffMap, specMap, viewDir);
    color += calculatePointLight(pointLight0, diffMap, specMap, viewDir);
    color += calculatePointLight(pointLight1, diffMap, specMap, viewDir);
    color += calculatePointLight(pointLight2, diffMap, specMap, viewDir);
    color += calculatePointLight(pointLight3, diffMap, specMap, viewDir);
    color += calculateSpotLight(spotLight, diffMap, specMap, viewDir);
    FragColor = vec4(color, 1.0);
}