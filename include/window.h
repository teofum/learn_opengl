#ifndef LEARN_OPENGL_WINDOW_H
#define LEARN_OPENGL_WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <camera.h>
#include <memory>

GLFWwindow *init_window(int initial_width, int initial_height, const char *title);

class Window {
private:
  bool init_success = false;

protected:
  GLFWwindow *glfw_window;
  std::unique_ptr<Camera> camera;

  float current_frame = 0.0f, delta_time = 0.0f;
  int viewport_width = -1, viewport_height = -1;

public:
  Window(int initial_width, int initial_height, const char *title);

  bool ready() const;

  void start();

  float aspect_ratio() const;

  virtual void setup() = 0;

  virtual void frame() = 0;

  virtual void resize_callback(int width, int height);

  virtual void mouse_callback(double x, double y);

  virtual void scroll_callback(double x, double y);

  virtual void key_callback(int key, int scancode, int action, int mods);

  virtual void process_input_sync();
};

#endif //LEARN_OPENGL_WINDOW_H
