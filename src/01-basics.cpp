#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

#include <program.h>
#include <texture.h>
#include <model.h>
#include <instance.h>
#include <camera.h>
#include <window.h>

#define WIDTH 800
#define HEIGHT 600

using namespace glm;

float ratio = 0.2f;

Camera *camera_ptr;
float delta_time = 0.0f;
float last_frame = 0.0f;

void key_callback(
  [[maybe_unused]] GLFWwindow *win,
  int key,
  [[maybe_unused]] int scancode,
  int action,
  [[maybe_unused]] int mods
) {
  if (action != GLFW_PRESS) return;

  switch (key) {
    case GLFW_KEY_X:
      ratio = fminf(ratio + 0.1f, 1.0f);
      break;
    case GLFW_KEY_Z:
      ratio = fmaxf(ratio - 0.1f, 0.0f);
      break;
    default:
      break;
  }
}

void mouse_callback([[maybe_unused]] GLFWwindow *window, double x_pos, double y_pos) {
  camera_ptr->process_mouse_input(x_pos, y_pos);
}

void scroll_callback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double x_offset, double y_offset) {
  camera_ptr->process_scroll_input(y_offset);
}

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  camera_ptr->process_keyboard_input(window, delta_time);
}

int main() {
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 01 — Basics");
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glEnable(GL_FRAMEBUFFER_SRGB);

  Camera camera;
  camera_ptr = &camera;

  // Compile shaders and link program
  // --------------------------------------------
  Program program("shaders/basics/vertex.glsl", "shaders/basics/fragment.glsl");

  camera.set_matrix_binding(program);

  // Setup vertex data
  // --------------------------------------------
  Model obj("assets/monkey.obj");
  Instance instance(obj, program);
  Instance instance2(obj, program);

  // Load tex_container image and generate texture
  // --------------------------------------------
  Texture tex_container("assets/container.jpg", Texture::Type::Specular);
  Texture tex_awesome("assets/awesome_face.png", Texture::Type::Specular);

  program.use();
  program.set("texture1", 0);
  program.set("texture2", 1);
  program.set("ratio", ratio);

  // Rendering loop
  // --------------------------------------------
  int width, height;
  while (!glfwWindowShouldClose(window)) {
    float current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);

    // Rendering code
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &width, &height);
    camera.update_matrices((float) width / (float) height);

    program.use();
    program.set("ratio", ratio);
    tex_container.bind(0);
    tex_awesome.bind(1);

    instance.transform = translate(mat4(1.0), vec3(0.75, 0.0, 0.0));
    instance.transform = rotate(instance.transform, radians(50.0f) * (float) glfwGetTime(), vec3(0.5f, 1.0f, 0.0f));
    instance.transform = scale(instance.transform, vec3(0.5f));
    instance.draw();

    instance2.transform = translate(mat4(1.0), vec3(-0.75, 0.0, 0.0));
    instance2.transform = scale(instance2.transform, vec3(0.5f));
    instance2.transform = rotate(instance2.transform, radians(-50.0f) * (float) glfwGetTime(), vec3(0.5f, 1.0f, 0.0f));
    instance2.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
