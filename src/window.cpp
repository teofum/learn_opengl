#include <window.h>

namespace {
void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
  auto window = (Window *) glfwGetWindowUserPointer(win);
  window->resize_callback(width, height);
}

void global_mouse_callback(GLFWwindow *win, double x_pos, double y_pos) {
  auto window = (Window *) glfwGetWindowUserPointer(win);
  window->mouse_callback(x_pos, y_pos);
}

void global_scroll_callback(GLFWwindow *win, double x_offset, double y_offset) {
  auto window = (Window *) glfwGetWindowUserPointer(win);
  window->scroll_callback(x_offset, y_offset);
}

void global_key_callback(GLFWwindow *win, int key, int scancode, int action, int mods) {
  auto window = (Window *) glfwGetWindowUserPointer(win);
  window->key_callback(key, scancode, action, mods);
}
}

GLFWwindow *init_window(int initial_width, int initial_height, const char *title) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create GLFW window
  GLFWwindow *window = glfwCreateWindow(
    initial_width, initial_height, title,
    nullptr, nullptr
  );
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwTerminate();
    return nullptr;
  }

  // Init viewport
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);

  return window;
}

Window::Window(int initial_width, int initial_height, const char *title) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create GLFW window
  glfw_window = glfwCreateWindow(
    initial_width, initial_height, title,
    nullptr, nullptr
  );
  if (!glfw_window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(glfw_window);
  glfwSetWindowUserPointer(glfw_window, this);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwTerminate();
    return;
  }

  camera = std::make_unique<Camera>(Camera());
  init_success = true;
}

bool Window::ready() const {
  return init_success;
}

void Window::start() {
  // Init viewport
  glfwGetFramebufferSize(glfw_window, &viewport_width, &viewport_height);
  glViewport(0, 0, viewport_width, viewport_height);

  glfwSetFramebufferSizeCallback(glfw_window, framebuffer_size_callback);
  glfwSetCursorPosCallback(glfw_window, global_mouse_callback);
  glfwSetScrollCallback(glfw_window, global_scroll_callback);
  glfwSetKeyCallback(glfw_window, global_key_callback);

  glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);

  setup();

  float last_frame = 0.0f;
  while (!glfwWindowShouldClose(glfw_window)) {
    current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    glfwGetFramebufferSize(glfw_window, &viewport_width, &viewport_height);

    process_input_sync();
    frame();

    glfwSwapBuffers(glfw_window);
    glfwPollEvents();
  }
}

void Window::resize_callback(int width, int height) {
  glViewport(0, 0, width, height);
}

void Window::mouse_callback(double x, double y) {
  camera->process_mouse_input(x, y);
}

void Window::scroll_callback(double x, double y) {
  camera->process_scroll_input(y);
}

void Window::key_callback(int key, int scancode, int action, int mods) {

}

void Window::process_input_sync() {
  if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(glfw_window, true);
  }

  camera->process_keyboard_input(glfw_window, delta_time);
}

float Window::aspect_ratio() const {
  return (float) viewport_width / (float) viewport_height;
}
