#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <program.h>
#include <object.h>
#include <instance.h>
#include <camera.h>
#include <window.h>
#include <texture.h>

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
  Object cube_obj("assets/cube.obj", cube_program);
  std::vector<Instance> cubes;

  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);

  for (int i = 0; i < 100; i++) {
    Instance cube(cube_obj, cube_program);
    cube.transform = translate(cube.transform, 5.0f * vec3(rnd(e1), rnd(e1), -0.5f - 0.5f * rnd(e1)));
    cube.transform = rotate(cube.transform, radians(360.0f * rnd(e1)), vec3(rnd(e1), rnd(e1), rnd(e1)));
    cubes.push_back(cube);
  }

  Object light_obj("assets/sphere.obj", light_program);
  Instance light(light_obj, light_program);

  Texture container("assets/container2.png", GL_RGBA);
  Texture container_spec("assets/container2_spec.png", GL_RGBA);

  // Setup uniforms
  // --------------------------------------------
//  vec3 light_pos(1.2f, 1.0f, 1.0f);
  vec3 light_color(1.0f);
  vec3 ambient = light_color * 0.1f;

  cube_program.use();
  glUniform1f(cube_program.uniform_location("light.innerAngle"), cos(radians(12.5f)));
  glUniform1f(cube_program.uniform_location("light.outerAngle"), cos(radians(15.0f)));
  glUniform3f(cube_program.uniform_location("light.ambient"), ambient.x, ambient.y, ambient.z);
  glUniform3f(cube_program.uniform_location("light.diffuse"), light_color.x, light_color.y, light_color.z);
  glUniform3f(cube_program.uniform_location("light.specular"), light_color.x, light_color.y, light_color.z);
  glUniform1f(cube_program.uniform_location("light.attConst"), 1.0f);
  glUniform1f(cube_program.uniform_location("light.attLinear"), 0.045f);
  glUniform1f(cube_program.uniform_location("light.attQuad"), 0.015f);

  glUniform1i(cube_program.uniform_location("material.diffuse"), 0);
  glUniform1i(cube_program.uniform_location("material.specular"), 1);
  glUniform1f(cube_program.uniform_location("material.shininess"), 32.0f);

//  light.transform = translate(mat4(1.0), light_pos);
//  light.transform = scale(light.transform, vec3(0.1));

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

    cube_program.use();
    glUniform3f(
      cube_program.uniform_location("light.position"),
      camera.position.x,
      camera.position.y,
      camera.position.z
    );
    glUniform3f(cube_program.uniform_location("light.direction"), camera.forward.x, camera.forward.y, camera.forward.z);
    glUniform3f(cube_program.uniform_location("viewPos"), camera.position.x, camera.position.y, camera.position.z);

    container.bind(0);
    container_spec.bind(1);
//    light.draw();

    for (const auto &cube: cubes) {
      cube.draw();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
