#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <program.h>
#include <model.h>
#include <instance.h>
#include <camera.h>
#include <window.h>
#include <light.h>
#include <random>

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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 03 — Model Loading");
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  camera.position = vec3(0.0, 1.5, 5.0);

  // Compile shaders and link cube_program
  // --------------------------------------------
  Shader vertex_shader = Shader::vertex("shaders/blending/vertex.glsl");
  Shader frag_base = Shader::fragment("shaders/blending/basic_frag.glsl");
  Shader frag_grass = Shader::fragment("shaders/blending/grass_frag.glsl");
  Shader frag_light = Shader::fragment("shaders/blending/light_frag.glsl");

  Program program(vertex_shader, frag_base);
  Program grass_program(vertex_shader, frag_grass);
  Program light_program(vertex_shader, frag_light);

  camera.add_program(&program);
  camera.add_program(&grass_program);
  camera.add_program(&light_program);

  // Setup objects
  // --------------------------------------------
  Model floor_model("assets/floor.obj");
  Instance floor(floor_model, program);

  floor.transform = scale(floor.transform, vec3(10.0f));

  Model box_model("assets/container.obj");
  Instance box1(box_model, program);
  Instance box2(box_model, program);

  box1.transform = translate(box1.transform, vec3(3.0f, 0.0f, 2.0f));
  box2.transform = translate(box2.transform, vec3(-1.0f, 0.0f, -1.0f));

  Model grass_model("assets/grass.obj");
  std::vector<Instance> grass;

  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);

  for (int i = 0; i < 20; i++) {
    Instance grass_instance(grass_model, grass_program);
    grass_instance.transform = translate(grass_instance.transform, 4.0f * vec3(rnd(e1), 0.0f, rnd(e1)));
    grass_instance.transform = rotate(grass_instance.transform, radians(360.0f * rnd(e1)), vec3(0.0, 1.0, 0.0));
    grass.push_back(grass_instance);
  }

  // Lights
  vec3 light_pos(1.2, 1.0, 0.2);
  PointLight light(light_pos);

  Model light_model("assets/sphere.obj");
  Instance light_ball(light_model, light_program);
  light_ball.transform = translate(mat4(1.0), light_pos);
  light_ball.transform = scale(light_ball.transform, vec3(0.1));

  // Setup uniforms
  // --------------------------------------------
  program.use();
  program.set("material.shininess", 32.0f);
  light.set_uniforms(program, "pointLight");

  grass_program.use();
  grass_program.set("material.shininess", 32.0f);
  light.set_uniforms(grass_program, "pointLight");

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
    floor.draw();
    box1.draw();
    box2.draw();
    for (const auto &grass_i: grass) grass_i.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
