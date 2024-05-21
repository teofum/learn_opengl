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
#define N_ASTEROIDS 100000

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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 06 — Skybox");
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Camera camera;
  camera_ptr = &camera;
  camera.position = vec3(0.0, 10.5, 20.0);

  // Compile shaders and link cube_program
  // --------------------------------------------
  Shader vertex_shader = Shader::vertex("shaders/instancing/vertex.glsl");
  Shader vertex_rock = Shader::vertex("shaders/instancing/vertex_rock.glsl");
  Shader fragment_shader = Shader::fragment("shaders/instancing/fragment.glsl");

  Program planet_program(vertex_shader, fragment_shader);
  Program asteroid_program(vertex_rock, fragment_shader);

  camera.set_matrix_binding(planet_program);
  camera.set_matrix_binding(asteroid_program);

  // Setup objects
  // --------------------------------------------
  Model planet_model("assets/planet/planet.obj");
  Instance planet(planet_model, planet_program);
  planet.transform = scale(planet.transform, vec3(4.0));

  std::random_device r;
  std::default_random_engine e1(r());

  Model asteroid("assets/rock/rock.obj");
  std::vector<mat4> asteroid_transforms;
  float radius = 75.0f;
  float offset = 25.0f;

  std::uniform_real_distribution<float> rnd_offset(-offset, offset);
  std::uniform_real_distribution<float> rnd_scale(0.05f, 0.25f);
  std::uniform_real_distribution<float> rnd_degrees(0.0f, 360.0f);
  std::uniform_real_distribution<float> rnd_1(-1.0f, 1.0f);

  for (int i = 0; i < N_ASTEROIDS; i++) {
    mat4 transform(1.0);

    float angle = (float) i / (float) N_ASTEROIDS * 360.0f;
    float x = sin(angle) * radius + rnd_offset(e1);
    float y = rnd_offset(e1) * 0.5f;
    float z = cos(angle) * radius + rnd_offset(e1);
    transform = translate(transform, vec3(x, y, z));

    transform = scale(transform, vec3(rnd_scale(e1)));

    float rot_angle = radians(rnd_degrees(e1));
    vec3 rot_axis = normalize(vec3(rnd_1(e1), rnd_1(e1), rnd_1(e1)));
    transform = rotate(transform, rot_angle, rot_axis);

    asteroid_transforms.push_back(transform);
  }

  asteroid.set_instance_attribute(3, asteroid_transforms);

  // Lights
  vec3 light_dir(-0.2f, -1.0f, 0.5f);
  DirectionalLight light(light_dir);

  // Setup uniforms
  // --------------------------------------------
  planet_program.use();
  planet_program.set("material.shininess", 32.0f);
  light.set_uniforms(planet_program, "directionalLight");

  asteroid_program.use();
  asteroid_program.set("material.shininess", 32.0f);
  light.set_uniforms(asteroid_program, "directionalLight");

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

    planet_program.use();
    planet_program.set("viewPos", camera.position);
    planet.draw();
    asteroid.draw_instanced(asteroid_program, N_ASTEROIDS);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
