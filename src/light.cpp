#include <light.h>

std::string Light::uname(const std::string &name, const std::string &prop) {
  std::stringstream ss;
  ss << name << "." << prop;
  return ss.str();
}

void DirectionalLight::set_uniforms(const Program &program, const std::string &light_name) const {
  program.set(uname(light_name, "direction").c_str(), direction);
  program.set(uname(light_name, "ambient").c_str(), ambient);
  program.set(uname(light_name, "diffuse").c_str(), diffuse);
  program.set(uname(light_name, "specular").c_str(), specular);
}

void DirectionalLight::set_matrix_binding(const Program &program) const {
  program.bind_uniform_block("LightMatrix", _binding_point);
}

void DirectionalLight::init_shadows(unsigned binding_point) {
  _binding_point = binding_point;
  glGenBuffers(1, &matrix_ubo);

  glBindBuffer(GL_UNIFORM_BUFFER, matrix_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4), nullptr, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, matrix_ubo);
}

void DirectionalLight::cast_shadows(const DepthFramebuffer &depth_buffer) const {
  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  depth_buffer.bind();
  glClear(GL_DEPTH_BUFFER_BIT);

  float depth = 10.0f;
  mat4 view = lookAt(direction * -depth * 0.5f, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
  mat4 projection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, depth);
  mat4 light_matrix = projection * view;

  glBindBuffer(GL_UNIFORM_BUFFER, matrix_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(light_matrix));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PointLight::set_uniforms(const Program &program, const std::string &light_name) const {
  program.set(uname(light_name, "position").c_str(), position);
  program.set(uname(light_name, "ambient").c_str(), ambient);
  program.set(uname(light_name, "diffuse").c_str(), diffuse);
  program.set(uname(light_name, "specular").c_str(), specular);
  program.set(uname(light_name, "attConst").c_str(), attenuation.x);
  program.set(uname(light_name, "attLinear").c_str(), attenuation.y);
  program.set(uname(light_name, "attQuad").c_str(), attenuation.z);
}

void SpotLight::set_uniforms(const Program &program, const std::string &light_name) const {
  program.set(uname(light_name, "position").c_str(), position);
  program.set(uname(light_name, "direction").c_str(), direction);
  program.set(uname(light_name, "innerAngle").c_str(), cos_inner_angle);
  program.set(uname(light_name, "outerAngle").c_str(), cos_outer_angle);
  program.set(uname(light_name, "ambient").c_str(), ambient);
  program.set(uname(light_name, "diffuse").c_str(), diffuse);
  program.set(uname(light_name, "specular").c_str(), specular);
  program.set(uname(light_name, "attConst").c_str(), attenuation.x);
  program.set(uname(light_name, "attLinear").c_str(), attenuation.y);
  program.set(uname(light_name, "attQuad").c_str(), attenuation.z);
}
