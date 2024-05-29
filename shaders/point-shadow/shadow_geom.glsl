#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
    mat4 lightMatrices[6];
};

layout (std140) uniform PointLightBlock {
    PointLight pointLight;
};

out vec4 fragPos;

void main() {
    for (int face = 0; face < 6; face++) {
        gl_Layer = face;
        for (int i = 0; i < 3; i++) {
            fragPos = gl_in[i].gl_Position;
            gl_Position = pointLight.lightMatrices[face] * fragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}