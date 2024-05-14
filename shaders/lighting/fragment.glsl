#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

uniform vec3 lightColor;
uniform vec3 lightPos;
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
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float attConst;
    float attLinear;
    float attQuad;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float outerAngle;
    float innerAngle;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float attConst;
    float attLinear;
    float attQuad;
};

uniform Material material;
uniform SpotLight light;

vec3 calculateDirectionalLight(DirectionalLight light, Material material, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    float diff = max(dot(-light.direction, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(light.direction, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light, Material material, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    float dist = length(light.position - fragPos);
    float attenuation = light.attConst + light.attLinear * dist + light.attQuad * dist * dist;

    return (ambient + diffuse + specular) / attenuation;
}

vec3 calculateSpotLight(SpotLight light, Material material, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, -light.direction);
    float cutoff = smoothstep(light.outerAngle, light.innerAngle, theta);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    float dist = length(light.position - fragPos);
    float attenuation = light.attConst + light.attLinear * dist + light.attQuad * dist * dist;

    return (ambient + (diffuse + specular) * cutoff) / attenuation;
}

void main() {
    vec3 diffMap = vec3(texture(material.diffuse, texCoord));
    vec3 specMap = vec3(texture(material.specular, texCoord));
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 color = calculateSpotLight(light, material, diffMap, specMap, viewDir);
    FragColor = vec4(color, 1.0);
}