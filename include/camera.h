#ifndef LEARN_OPENGL_CAMERA_H
#define LEARN_OPENGL_CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include <vector>

#include <program.h>

using namespace glm;

class Camera {
private:
  vec2 last_frame_cursor = vec2(0.0, 0.0);
  bool mouse_moved = false;
  unsigned matrix_ubo, binding_point;

public:
  vec3 position;
  vec3 forward = vec3(0.0f, 0.0f, -1.0f);
  vec3 up = vec3(0.0f, 1.0f, 0.0f);
  vec2 angles;
  float fov;

  float zoom_speed = 1.0f;
  float camera_speed = 2.5f;
  float mouse_sensitivity = 0.1f;

  explicit Camera(
    vec3 position = vec3(0.0f, 0.0f, 3.0f),
    float fov = 45.0f,
    vec2 angles = vec2(-90.0f, 0.0f),
    unsigned binding_point = 0
  );

  mat4 get_view_matrix() const;

  mat4 get_projection_matrix(float aspect) const;

  void process_mouse_input(double x_pos, double y_pos);

  void process_scroll_input(double y_offset);

  void process_keyboard_input(GLFWwindow *window, float delta_time);

  void set_matrix_binding(const Program &program) const;

  void update_matrices(float aspect) const;
};

#endif //LEARN_OPENGL_CAMERA_H
