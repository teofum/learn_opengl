#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <program.h>
#include <model.h>
#include <instance.h>
#include <camera.h>
#include <window.h>
#include <light.h>

#define WIDTH 800
#define HEIGHT 600

using namespace glm;

Camera *camera_ptr;
float delta_time = 0.0f;
float last_frame = 0.0f;

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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 03 — Model Loading");
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glEnable(GL_FRAMEBUFFER_SRGB);

  Camera camera;
  camera_ptr = &camera;
  camera.position = vec3(0.0, 0.0, 5.0);

  // Compile shaders and link cube_program
  // --------------------------------------------
  Shader vertex_shader = Shader::vertex("shaders/model/vertex.glsl");
  Shader fragment_shader = Shader::fragment("shaders/model/fragment.glsl");
  Shader light_shader = Shader::fragment("shaders/model/light_frag.glsl");

  Program program(vertex_shader, fragment_shader);
  Program light_program(vertex_shader, light_shader);

  camera.set_matrix_binding(program);
  camera.set_matrix_binding(light_program);

  // Setup objects
  // --------------------------------------------
  Model model("assets/backpack/backpack.obj");
  Instance object(model, program);

  // Lights
  vec3 light_pos(1.2, 1.0, 0.2);
  PointLight light(1, light_pos);

  Model light_model("assets/sphere.obj");
  Instance light_ball(light_model, light_program);
  light_ball.transform = translate(mat4(1.0), light_pos);
  light_ball.transform = scale(light_ball.transform, vec3(0.1));

  // Setup uniforms
  // --------------------------------------------
  program.use();
  program.set("material.shininess", 32.0f);
  light.set_ubo_binding(program, "PointLightBlock");
  light.update_ubo();

  // Rendering loop
  // --------------------------------------------
  int width, height;
  while (!glfwWindowShouldClose(window)) {
    float current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);

    // Rendering code
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &width, &height);
    camera.update_matrices((float) width / (float) height);

    program.use();
    program.set("viewPos", camera.position);

    light_ball.draw();
    object.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
