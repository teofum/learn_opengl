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

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirectionalLight directionalLight;

vec3 calculateDirectionalLight(DirectionalLight light, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    float diff = max(dot(-light.direction, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(light.direction, normal);
    float spec = pow(max(dot(viewDir, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;

    return ambient + diffuse + specular;
}

void main() {
    vec4 diff = texture(material.diffuse0, texCoord);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 color = vec3(0.0);
    color += calculateDirectionalLight(directionalLight, vec3(diff), vec3(1.0f), viewDir);
    FragColor = vec4(color, diff.a);
}