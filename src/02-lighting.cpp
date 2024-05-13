#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <program.h>
#include <object.h>
#include <instance.h>
#include <camera.h>
#include <window.h>

#define WIDTH 800
#define HEIGHT 600

using namespace glm;

Camera camera;
float delta_time = 0.0f;
float last_frame = 0.0f;

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

int main() {
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 02 — Lighting");

  // Compile shaders and link program
  // --------------------------------------------
  Program program("shaders/lighting/vertex.glsl", "shaders/lighting/fragment.glsl");

  // Setup vertex data
  // --------------------------------------------
  Object obj("assets/monkey.obj", program);
  Instance instance(obj, program);

  program.use();

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

    mat4 view = camera.get_view_matrix();
    int loc_view = program.uniform_location("view");
    glUniformMatrix4fv(loc_view, 1, GL_FALSE, value_ptr(view));

    glfwGetFramebufferSize(window, &width, &height);
    mat4 projection = camera.get_projection_matrix((float) width / (float) height);
    int loc_projection = program.uniform_location("projection");
    glUniformMatrix4fv(loc_projection, 1, GL_FALSE, value_ptr(projection));

    program.use();
    instance.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
