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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 08 — Shadow mapping");
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
  Program program("shaders/shadow-map/vertex.glsl", "shaders/shadow-map/fragment.glsl");
  Program shadow_program("shaders/shadow-map/shadow_vert.glsl", "shaders/shadow-map/shadow_frag.glsl");
  Program skybox_program("shaders/skybox/sky_vert.glsl", "shaders/skybox/sky_frag.glsl");

  camera.set_matrix_binding(program);
  camera.set_matrix_binding(skybox_program);

  // Setup objects
  // --------------------------------------------
  Model floor_model("assets/floor.obj");
  Instance floor(floor_model, program);

  floor.transform = scale(floor.transform, vec3(10.0f));

  Model box_model("assets/container.obj");
  Instance box1(box_model, program);
  Instance box2(box_model, program);

  box1.transform = translate(box1.transform, vec3(3.0f, 0.0f, 2.0f));

  box2.transform = translate(box2.transform, vec3(-1.0f, 1.0f, -1.0f));
  box2.transform = rotate(box2.transform, radians(55.0f), vec3(-1.0f, 1.0f, -1.0f));

  // Lights
  vec3 light_dir(-0.2f, -1.0f, 0.5f);
  DirectionalLight light(1, light_dir);
  light.set_ubo_binding(program, "DirectionalLightBlock");
  light.set_ubo_binding(shadow_program, "DirectionalLightBlock");

  // Setup skybox
  // --------------------------------------------
  std::vector<Vertex> sky_vertices = {
    {vec3(-1.0f, -1.0f, -1.0f), vec3(), vec3(), vec2()},
    {vec3(-1.0f, -1.0f, 1.0f),  vec3(), vec3(), vec2()},
    {vec3(-1.0f, 1.0f, -1.0f),  vec3(), vec3(), vec2()},
    {vec3(-1.0f, 1.0f, 1.0f),   vec3(), vec3(), vec2()},
    {vec3(1.0f, -1.0f, -1.0f),  vec3(), vec3(), vec2()},
    {vec3(1.0f, -1.0f, 1.0f),   vec3(), vec3(), vec2()},
    {vec3(1.0f, 1.0f, -1.0f),   vec3(), vec3(), vec2()},
    {vec3(1.0f, 1.0f, 1.0f),    vec3(), vec3(), vec2()}
  };
  std::vector<unsigned> sky_indices = {
    2, 0, 4, 4, 6, 2,
    1, 0, 2, 2, 3, 1,
    4, 5, 7, 7, 6, 4,
    1, 3, 7, 7, 5, 1,
    2, 6, 7, 7, 3, 2,
    0, 1, 4, 4, 1, 5
  };
  std::vector<Texture> sky_textures;
  Mesh skybox(std::move(sky_vertices), std::move(sky_indices), std::move(sky_textures));

  const char *skybox_paths[6] = {
    "assets/skybox/right.jpg",
    "assets/skybox/left.jpg",
    "assets/skybox/top.jpg",
    "assets/skybox/bottom.jpg",
    "assets/skybox/front.jpg",
    "assets/skybox/back.jpg"
  };
  Texture skybox_texture(skybox_paths, Texture::Type::Diffuse);

  // Setup uniforms
  // --------------------------------------------
  program.use();
  program.set("skybox", 10);
  program.set("shadowMap", 11);
  program.set("material.shininess", 32.0f);

  skybox_program.use();
  skybox_program.set("skybox", 0);

  // Setup shadow framebuffer
  // --------------------------------------------
  DepthFramebuffer shadow_depth_buffer(SHADOW_WIDTH, SHADOW_HEIGHT);

  // Rendering loop
  // --------------------------------------------
  int width, height;
  glDepthFunc(GL_LEQUAL);
  while (!glfwWindowShouldClose(window)) {
    float current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);
    glfwGetFramebufferSize(window, &width, &height);

    // Update light
    light.direction = vec3(cos(current_frame) * 0.5f, -1.0f, sin(current_frame) * 0.5f);
    program.use();
    light.update_ubo();

    // Shadow mapping
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    shadow_depth_buffer.bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    floor.draw_with(shadow_program);
    box1.draw_with(shadow_program);
    box2.draw_with(shadow_program);
    Framebuffer::unbind();

    // Rendering code
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.update_matrices((float) width / (float) height);

    program.use();
    program.set("viewPos", camera.position);
    skybox_texture.bind(10);
    glActiveTexture(GL_TEXTURE0 + 11);
    glBindTexture(GL_TEXTURE_2D, shadow_depth_buffer.depth_map());

    floor.draw();
    box1.draw();
    box2.draw();

    glDepthMask(GL_FALSE);
    skybox_program.use();
    skybox_texture.bind();
    skybox.draw(skybox_program);
    glDepthMask(GL_TRUE);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
