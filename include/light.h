#ifndef LEARN_OPENGL_LIGHT_H
#define LEARN_OPENGL_LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <program.h>
#include <framebuffer.h>

#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

using namespace glm;

class Light {
protected:
  unsigned ubo;

public:
  vec3 ambient, diffuse, specular;
  unsigned binding_point = 0;

  Light(unsigned binding_point, unsigned ubo_size, vec3 ambient, vec3 diffuse, vec3 specular);

  void set_ubo_binding(const Program &program, const char *block_name) const;

  virtual void update_ubo() const = 0;
};

class DirectionalLight : public Light {
public:
  vec3 direction;

  DirectionalLight(unsigned binding_point, vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular)
    : Light(binding_point, sizeof(mat4) + 4 * sizeof(vec4), ambient, diffuse, specular), direction(direction) {
  }

  explicit DirectionalLight(
    unsigned binding_point,
    vec3 direction,
    vec3 color = vec3(1.0f),
    float ambient_intensity = 0.1f
  )
    : DirectionalLight(binding_point, direction, color * ambient_intensity, color, color) {
  }

  void update_ubo() const override;
};

class PointLight : public Light {
public:
  vec3 position, attenuation;

  PointLight(
    unsigned binding_point,
    vec3 position,
    vec3 ambient,
    vec3 diffuse,
    vec3 specular,
    vec3 attenuation = vec3(0.0, 0.0, 1.0))
    : Light(binding_point, sizeof(mat4) + 5 * sizeof(vec4), ambient, diffuse, specular), position(position),
      attenuation(attenuation) {
  }

  explicit PointLight(unsigned binding_point, vec3 position, vec3 color = vec3(1.0f), float ambient_intensity = 0.1f)
    : PointLight(binding_point, position, color * ambient_intensity, color, color) {
  }

  void update_ubo() const override;
};

class SpotLight : public Light {
public:
  vec3 position, direction, attenuation;
  float cos_inner_angle, cos_outer_angle;

  SpotLight(
    unsigned binding_point,
    vec3 position,
    vec3 direction,
    float inner_angle,
    float outer_angle,
    vec3 ambient,
    vec3 diffuse,
    vec3 specular,
    vec3 attenuation = vec3(0.0, 0.0, 1.0)
  )
    : Light(binding_point, sizeof(mat4) + 6 * sizeof(vec4) + 2 * sizeof(float), ambient, diffuse, specular),
      position(position),
      direction(direction),
      cos_inner_angle(cos(inner_angle)),
      cos_outer_angle(cos(outer_angle)),
      attenuation(attenuation) {
  }

  SpotLight(
    unsigned binding_point,
    vec3 position,
    vec3 direction,
    float inner_angle,
    float outer_angle,
    vec3 color = vec3(1.0f),
    float ambient_intensity = 0.1f
  )
    : SpotLight(binding_point, position, direction, inner_angle, outer_angle, color * ambient_intensity, color, color) {
  }

  SpotLight(
    unsigned binding_point,
    vec3 position,
    vec3 direction,
    float inner_angle,
    vec3 color = vec3(1.0f),
    float ambient_intensity = 0.1f
  )
    : SpotLight(binding_point, position, direction, inner_angle, inner_angle * 1.2f, color, ambient_intensity) {
  }

  void update_ubo() const override;
};

#endif //LEARN_OPENGL_LIGHT_H
