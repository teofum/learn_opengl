#include <camera.h>

mat4 Camera::get_view_matrix() const {
  return lookAt(position, position + forward, up);
}

mat4 Camera::get_projection_matrix(float aspect) const {
  return perspective(radians(fov), aspect, 0.1f, 100.0f);
}

void Camera::process_mouse_input(double x_pos, double y_pos) {
  vec2 cursor(x_pos, y_pos);

  if (mouse_moved) {
    vec2 delta = (cursor - last_frame_cursor) * vec2(1.0, -1.0) * mouse_sensitivity;
    angles += delta;
    angles.y = clamp(angles.y, -89.0f, 89.0f);

    forward.x = cos(radians(angles.x)) * cos(radians(angles.y));
    forward.y = sin(radians(angles.y));
    forward.z = sin(radians(angles.x)) * cos(radians(angles.y));
    forward = normalize(forward);
  }

  last_frame_cursor = cursor;
  mouse_moved = true;
}

void Camera::process_scroll_input(double y_offset) {
  fov = clamp(fov - (float) y_offset * zoom_speed, 6.5f, 65.0f);
}

void Camera::process_keyboard_input(GLFWwindow *window, float delta_time) {
  float speed = camera_speed * delta_time;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    position += forward * speed;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    position -= forward * speed;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    position -= normalize(cross(forward, up)) * speed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    position += normalize(cross(forward, up)) * speed;
}

void Camera::add_program(Program *program) {
  programs.push_back(program);
}

void Camera::update_matrices(float aspect) {
  mat4 view = get_view_matrix();
  mat4 projection = get_projection_matrix(aspect);

  for (auto program: programs) {
    program->use();
    program->set_matrix("view", view);
    program->set_matrix("projection", projection);
  }
}
