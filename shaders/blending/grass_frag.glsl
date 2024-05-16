#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse0;
    float shininess;
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

uniform Material material;
uniform PointLight pointLight;

vec3 calculatePointLight(PointLight light, vec3 diffMap, vec3 specMap, vec3 viewDir) {
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

void main() {
    vec4 diff = texture(material.diffuse0, texCoord);
    vec3 viewDir = normalize(viewPos - fragPos);

    if (diff.a < 0.1) discard;

    vec3 color = vec3(0.0);
    color += calculatePointLight(pointLight, vec3(diff), vec3(0.5f), viewDir);
    FragColor = vec4(color, 1.0);
}