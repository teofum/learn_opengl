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

void mouse_callback([[maybe_unused]] GLFWwindow *window, double x_pos, double y_pos) {
  camera.process_mouse_input(x_pos, y_pos);
}

void scroll_callback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double x_offset, double y_offset) {
  camera.process_scroll_input(y_offset);
}

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  camera.process_keyboard_input(window, delta_time);
}

int main() {
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 02 — Lighting");
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  camera.position = vec3(0.0, 0.0, 5.0);

  // Compile shaders and link cube_program
  // --------------------------------------------
  Shader vertex_shader = Shader::vertex("shaders/lighting/vertex.glsl");
  Shader fragment_shader = Shader::fragment("shaders/lighting/fragment.glsl");
  Shader light_shader = Shader::fragment("shaders/lighting/light_frag.glsl");

  Program cube_program(vertex_shader, fragment_shader);
  Program light_program(vertex_shader, light_shader);

  camera.add_program(&cube_program);
  camera.add_program(&light_program);

  // Setup objects
  // --------------------------------------------
  Object cube_obj("assets/monkey.obj", cube_program);
  Instance cube(cube_obj, cube_program);

  Object light_obj("assets/sphere.obj", light_program);
  Instance light(light_obj, light_program);

  // Setup uniforms
  // --------------------------------------------
  vec3 light_pos(1.2f, 1.0f, 1.0f);

  cube_program.use();
  glUniform3f(cube_program.uniform_location("lightPos"), light_pos.x, light_pos.y, light_pos.z);
  glUniform3f(cube_program.uniform_location("objectColor"), 1.0f, 0.5f, 0.31f);
  glUniform3f(cube_program.uniform_location("lightColor"), 1.0f, 1.0f, 1.0f);

  light.transform = translate(mat4(1.0), light_pos);
  light.transform = scale(light.transform, vec3(0.1));

  cube.transform = scale(mat4(1.0), vec3(0.5));

  // Rendering loop
  // --------------------------------------------
  int width, height;
  while (!glfwWindowShouldClose(window)) {
    float current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);

    // Rendering code
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &width, &height);
    camera.update_matrices((float) width / (float) height);

    cube_program.use();
    glUniform3f(cube_program.uniform_location("viewPos"), camera.position.x, camera.position.y, camera.position.z);

    cube.draw();
    light.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
