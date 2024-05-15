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
