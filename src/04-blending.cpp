#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <program.h>
#include <model.h>
#include <instance.h>
#include <camera.h>
#include <window.h>
#include <light.h>
#include <random>
#include <map>
#include <ranges>

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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 04 — Blending");
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_FRAMEBUFFER_SRGB);

  Camera camera;
  camera_ptr = &camera;
  camera.position = vec3(0.0, 1.5, 5.0);

  // Compile shaders and link cube_program
  // --------------------------------------------
  Shader vertex_shader = Shader::vertex("shaders/blending/vertex.glsl");
  Shader frag_base = Shader::fragment("shaders/blending/basic_frag.glsl");
  Shader frag_grass = Shader::fragment("shaders/blending/grass_frag.glsl");
  Shader frag_window = Shader::fragment("shaders/blending/window_frag.glsl");
  Shader frag_light = Shader::fragment("shaders/blending/light_frag.glsl");

  Program program(vertex_shader, frag_base);
  Program grass_program(vertex_shader, frag_grass);
  Program window_program(vertex_shader, frag_window);
  Program light_program(vertex_shader, frag_light);

  camera.set_matrix_binding(program);
  camera.set_matrix_binding(grass_program);
  camera.set_matrix_binding(window_program);
  camera.set_matrix_binding(light_program);

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
  grass_model.cull_backfaces = false;
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

  Model window_model("assets/window.obj");
  window_model.cull_backfaces = false;
  std::vector<Instance> windows;

  for (int i = 0; i < 6; i++) {
    Instance window_instance(window_model, window_program);
    window_instance.transform = translate(window_instance.transform, 2.0f * vec3(rnd(e1), 0.0f, rnd(e1)));
    windows.push_back(window_instance);
  }

  // Lights
  vec3 light_dir(-0.2f, -1.0f, 0.0f);
  DirectionalLight light(light_dir);

  // Setup uniforms
  // --------------------------------------------
  program.use();
  program.set("material.shininess", 32.0f);
  light.set_uniforms(program, "directionalLight");

  grass_program.use();
  grass_program.set("material.shininess", 32.0f);
  light.set_uniforms(grass_program, "directionalLight");

  window_program.use();
  window_program.set("material.shininess", 32.0f);
  light.set_uniforms(window_program, "directionalLight");

  // Rendering loop
  // --------------------------------------------
  int width, height;
  while (!glfwWindowShouldClose(window)) {
    float current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);

    // Rendering code
    glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &width, &height);
    camera.update_matrices((float) width / (float) height);

    program.use();
    program.set("viewPos", camera.position);

    floor.draw();
    box1.draw();
    box2.draw();
    for (const auto &grass_i: grass) grass_i.draw();

    // Sort windows by distance to camera
    std::map<float, const Instance *> sorted_windows;
    for (const auto &window_i: windows) {
      vec3 position(column(window_i.transform, 3));
      vec3 diff = position - camera.position;
      float dist = dot(diff, normalize(camera.forward));
      sorted_windows[dist] = &window_i;
    }
    for (auto &window_i: std::ranges::reverse_view(sorted_windows)) {
      window_i.second->draw();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
