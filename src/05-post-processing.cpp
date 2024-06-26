#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <program.h>
#include <model.h>
#include <instance.h>
#include <camera.h>
#include <window.h>
#include <light.h>
#include <framebuffer.h>

#define WIDTH 800
#define HEIGHT 600

using namespace glm;

Camera *camera_ptr;
float delta_time = 0.0f;
float last_frame = 0.0f;

unsigned effect = 0, effect_count = 1;

void key_callback(
  [[maybe_unused]] GLFWwindow *win,
  int key,
  [[maybe_unused]] int scancode,
  int action,
  [[maybe_unused]] int mods
) {
  if (action != GLFW_PRESS) return;

  if (key == GLFW_KEY_SPACE)
    effect = (effect + 1) % effect_count;
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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 05 — Framebuffer");
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Camera camera;
  camera_ptr = &camera;
  camera.position = vec3(0.0, 1.5, 5.0);

  // Compile shaders and link cube_program
  // --------------------------------------------
  Shader vertex_shader = Shader::vertex("shaders/post-processing/vertex.glsl");
  Shader frag_base = Shader::fragment("shaders/post-processing/basic_frag.glsl");
  Shader frag_light = Shader::fragment("shaders/post-processing/light_frag.glsl");

  Program program(vertex_shader, frag_base);
  Program light_program(vertex_shader, frag_light);

  camera.set_matrix_binding(program);
  camera.set_matrix_binding(light_program);

  Shader pp_vertex = Shader::vertex("shaders/post-processing/pp_vertex.glsl");
  std::vector<const Program *> post_programs;

  Shader pp_frag_id = Shader::fragment("shaders/post-processing/pp_frag_id.glsl");
  Program program_id(pp_vertex, pp_frag_id);
  post_programs.push_back(&program_id);
  Shader pp_frag_invert = Shader::fragment("shaders/post-processing/pp_frag_invert.glsl");
  Program program_invert(pp_vertex, pp_frag_invert);
  post_programs.push_back(&program_invert);
  Shader pp_frag_gray = Shader::fragment("shaders/post-processing/pp_frag_gray.glsl");
  Program program_gray(pp_vertex, pp_frag_gray);
  post_programs.push_back(&program_gray);
  Shader pp_frag_blur = Shader::fragment("shaders/post-processing/pp_frag_blur.glsl");
  Program program_blur(pp_vertex, pp_frag_blur);
  post_programs.push_back(&program_blur);
  Shader pp_frag_sharpen = Shader::fragment("shaders/post-processing/pp_frag_sharpen.glsl");
  Program program_sharpen(pp_vertex, pp_frag_sharpen);
  post_programs.push_back(&program_sharpen);
  Shader pp_frag_edge_detect = Shader::fragment("shaders/post-processing/pp_frag_edge_detect.glsl");
  Program program_edge_detect(pp_vertex, pp_frag_edge_detect);
  post_programs.push_back(&program_edge_detect);

  effect_count = post_programs.size();

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

  // Lights
  vec3 light_dir(-0.2f, -1.0f, 0.5f);
  DirectionalLight light(1, light_dir);
  light.update_ubo();

  // Setup uniforms
  // --------------------------------------------
  program.use();
  program.set("material.shininess", 32.0f);
  light.set_ubo_binding(program, "DirectionalLightBlock");

  // Setup framebuffer
  // --------------------------------------------
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  TextureFramebuffer fb(width, height);

  // Setup screen quad
  // --------------------------------------------
  std::vector<Vertex> quad_vertices = {
    {vec3(-1.0f, 1.0f, 0.0f),  vec3(), vec3(), vec2(0.0f, 1.0f)},
    {vec3(-1.0f, -1.0f, 0.0f), vec3(), vec3(), vec2(0.0f, 0.0f)},
    {vec3(1.0f, -1.0f, 0.0f),  vec3(), vec3(), vec2(1.0f, 0.0f)},
    {vec3(1.0f, 1.0f, 0.0f),   vec3(), vec3(), vec2(1.0f, 1.0f)}
  };
  std::vector<unsigned> quad_indices = {0, 1, 2, 0, 2, 3};
  std::vector<Texture> quad_textures;
  Mesh screen_quad(std::move(quad_vertices), std::move(quad_indices), std::move(quad_textures));

  // Rendering loop
  // --------------------------------------------
  while (!glfwWindowShouldClose(window)) {
    float current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);

    // Rendering code
    fb.bind();
    glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &width, &height);
    camera.update_matrices((float) width / (float) height);

    program.use();
    program.set("viewPos", camera.position);

    floor.draw();
    box1.draw();
    box2.draw();
    TextureFramebuffer::unbind();

    // Post processing pass
    const Program *post_program = post_programs[effect];
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    post_program->use();
    post_program->set("screenWidth", width);
    post_program->set("screenHeight", height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb.texture());
    screen_quad.draw(*post_program);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
