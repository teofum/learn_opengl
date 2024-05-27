#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec4 fragPosLightSpace;

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse0;
    sampler2D specular0;
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
uniform samplerCube skybox;
uniform sampler2D shadowMap;

float calculateShadow(DirectionalLight light) {
    vec3 lightSpace = fragPosLightSpace.xyz / fragPosLightSpace.w;
    lightSpace = lightSpace * 0.5 + 0.5;
    float bias = max(0.05 * (1.0 - dot(normal, -light.direction)), 0.005);
    float fragDepth = lightSpace.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closestDepth = texture(shadowMap, lightSpace.xy + vec2(x, y) * texelSize).r;
            shadow += step(fragDepth - bias, closestDepth);
        }
    }
    shadow /= 9.0;
    if (fragDepth >= 1.0) shadow = 1.0;

    return shadow;
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 diffMap, vec3 specMap, vec3 viewDir) {
    vec3 ambient = diffMap * light.ambient;

    float diff = max(dot(-light.direction, normal), 0.0);
    vec3 diffuse = diff * diffMap * light.diffuse;

    vec3 reflectionDir = reflect(viewDir, normal);
    float spec = pow(max(dot(-light.direction, reflectionDir), 0.0), material.shininess);
    vec3 specular = specMap * spec * light.specular;
    vec3 reflection = specMap * vec3(texture(skybox, reflectionDir));

    float shadow = calculateShadow(light);

    return ambient + (diffuse + specular) * shadow;
}

void main() {
    vec3 diffMap = vec3(texture(material.diffuse0, texCoord));
    vec3 specMap = vec3(texture(material.specular0, texCoord));
    vec3 viewDir = normalize(fragPos - viewPos);

    vec3 color = vec3(0.0);
    color += calculateDirectionalLight(directionalLight, diffMap, specMap, viewDir);
    FragColor = vec4(color, 1.0);
}