#include <window.h>

namespace {
void framebuffer_size_callback([[maybe_unused]] GLFWwindow *win, int width, int height) {
  glViewport(0, 0, width, height);
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
