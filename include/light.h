#ifndef LEARN_OPENGL_LIGHT_H
#define LEARN_OPENGL_LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <program.h>

using namespace glm;

class Light {
protected:
  static std::string uname(const std::string &name, const std::string &prop);

public:
  vec3 ambient, diffuse, specular;

  Light(vec3 ambient, vec3 diffuse, vec3 specular)
    : ambient(ambient), diffuse(diffuse), specular(specular) {
  }

  explicit Light(vec3 color = vec3(1.0f), float ambient_intensity = 0.1f)
    : Light(color * ambient_intensity, color, color) {
  }

  [[maybe_unused]] virtual void set_uniforms(const Program &program, const std::string &light_name) const = 0;
};

class DirectionalLight : Light {
public:
  vec3 direction;

  DirectionalLight(vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular)
    : Light(ambient, diffuse, specular), direction(direction) {
  }

  explicit DirectionalLight(vec3 direction, vec3 color = vec3(1.0f), float ambient_intensity = 0.1f)
    : DirectionalLight(direction, color * ambient_intensity, color, color) {
  }

  void set_uniforms(const Program &program, const std::string &light_name) const override;
};

class PointLight : Light {
public:
  vec3 position, attenuation;

  PointLight(vec3 position, vec3 ambient, vec3 diffuse, vec3 specular, vec3 attenuation = vec3(1.0, 0.09, 0.032))
    : Light(ambient, diffuse, specular), position(position), attenuation(attenuation) {
  }

  explicit PointLight(vec3 position, vec3 color = vec3(1.0f), float ambient_intensity = 0.1f)
    : PointLight(position, color * ambient_intensity, color, color) {
  }

  void set_uniforms(const Program &program, const std::string &light_name) const override;
};

class SpotLight : Light {
public:
  vec3 position, direction, attenuation;
  float cos_inner_angle, cos_outer_angle;

  SpotLight(
    vec3 position,
    vec3 direction,
    float inner_angle,
    float outer_angle,
    vec3 ambient,
    vec3 diffuse,
    vec3 specular,
    vec3 attenuation = vec3(1.0, 0.09, 0.032)
  )
    : Light(ambient, diffuse, specular),
      position(position),
      direction(direction),
      cos_inner_angle(cos(inner_angle)),
      cos_outer_angle(cos(outer_angle)),
      attenuation(attenuation) {
  }

  SpotLight(
    vec3 position,
    vec3 direction,
    float inner_angle,
    float outer_angle,
    vec3 color = vec3(1.0f),
    float ambient_intensity = 0.1f
  )
    : SpotLight(position, direction, inner_angle, outer_angle, color * ambient_intensity, color, color) {
  }

  SpotLight(
    vec3 position,
    vec3 direction,
    float inner_angle,
    vec3 color = vec3(1.0f),
    float ambient_intensity = 0.1f
  )
    : SpotLight(position, direction, inner_angle, inner_angle * 1.2f, color, ambient_intensity) {
  }

  void set_uniforms(const Program &program, const std::string &light_name) const override;
};

#endif //LEARN_OPENGL_LIGHT_H
