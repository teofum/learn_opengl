#include <light.h>

Light::Light(unsigned binding_point, unsigned ubo_size, vec3 ambient, vec3 diffuse, vec3 specular)
  : ambient(ambient), diffuse(diffuse), specular(specular), ubo(0), binding_point(binding_point) {
  glGenBuffers(1, &ubo);

  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, ubo_size, nullptr, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, ubo);
}

void Light::set_ubo_binding(const Program &program, const char *block_name) const {
  program.bind_uniform_block(block_name, binding_point);
}

void DirectionalLight::update_ubo() const {
  float depth = 10.0f;
  mat4 view = lookAt(direction * -depth * 0.5f, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
  mat4 projection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, depth);
  mat4 light_matrix = projection * view;

  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0 * sizeof(vec4), sizeof(vec4), value_ptr(direction));
  glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(vec4), sizeof(vec4), value_ptr(ambient));
  glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(vec4), sizeof(vec4), value_ptr(diffuse));
  glBufferSubData(GL_UNIFORM_BUFFER, 3 * sizeof(vec4), sizeof(vec4), value_ptr(specular));
  glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(vec4), sizeof(mat4), value_ptr(light_matrix));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PointLight::update_ubo() const {
  // TODO
//  float depth = 10.0f;
//  mat4 view = lookAt(position, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
//  mat4 projection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, depth);
//  mat4 light_matrix = projection * view;

  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
//  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(light_matrix));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 0 * sizeof(vec4), sizeof(vec3), value_ptr(position));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 1 * sizeof(vec4), sizeof(vec3), value_ptr(ambient));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 2 * sizeof(vec4), sizeof(vec3), value_ptr(diffuse));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 3 * sizeof(vec4), sizeof(vec3), value_ptr(specular));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 4 * sizeof(vec4), sizeof(vec3), value_ptr(attenuation));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SpotLight::update_ubo() const {
  float depth = 10.0f;
  mat4 view = lookAt(position, position + direction, vec3(0.0f, 1.0f, 0.0f));
  mat4 projection = perspective(acos(cos_outer_angle), 1.0f, 1.0f, depth);
  mat4 light_matrix = projection * view;

  vec2 angles(cos_outer_angle, cos_inner_angle);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(light_matrix));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 0 * sizeof(vec4), sizeof(vec3), value_ptr(position));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 1 * sizeof(vec4), sizeof(vec3), value_ptr(direction));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 2 * sizeof(vec4), sizeof(vec3), value_ptr(ambient));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 3 * sizeof(vec4), sizeof(vec3), value_ptr(diffuse));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 4 * sizeof(vec4), sizeof(vec3), value_ptr(specular));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 5 * sizeof(vec4), sizeof(vec3), value_ptr(attenuation));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4) + 6 * sizeof(vec4), sizeof(vec2), value_ptr(angles));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
