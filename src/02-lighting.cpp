#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <program.h>
#include <model.h>
#include <instance.h>
#include <camera.h>
#include <window.h>
#include <texture.h>
#include <light.h>

#define WIDTH 800
#define HEIGHT 600
#define N_POINT_LIGHTS 4

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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 02 — Lighting");
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glEnable(GL_FRAMEBUFFER_SRGB);

  Camera camera;
  camera_ptr = &camera;
  camera.position = vec3(0.0, 0.0, 5.0);

  // Compile shaders and link cube_program
  // --------------------------------------------
  Shader vertex_shader = Shader::vertex("shaders/lighting/vertex.glsl");
  Shader fragment_shader = Shader::fragment("shaders/lighting/fragment.glsl");
  Shader light_shader = Shader::fragment("shaders/lighting/light_frag.glsl");

  Program cube_program(vertex_shader, fragment_shader);
  Program light_program(vertex_shader, light_shader);

  camera.set_matrix_binding(cube_program);
  camera.set_matrix_binding(light_program);

  // Setup objects
  // --------------------------------------------
  Model cube_obj("assets/cube.obj");
  std::vector<Instance> cubes;

  std::random_device r;
  std::default_random_engine e1(r());
  std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);

  for (int i = 0; i < 20; i++) {
    Instance cube(cube_obj, cube_program);
    cube.transform = translate(cube.transform, 5.0f * vec3(rnd(e1), rnd(e1), -0.5f - 0.5f * rnd(e1)));
    cube.transform = rotate(cube.transform, radians(360.0f * rnd(e1)), vec3(rnd(e1), rnd(e1), rnd(e1)));
    cubes.push_back(cube);
  }

  // Lights
  DirectionalLight dir_light(1, vec3(-0.2, -1.0, -0.2), vec3(0.5f));

  Model light_obj("assets/sphere.obj");
  std::vector<Instance> lights;
  std::vector<PointLight> point_lights;

  vec3 light_positions[N_POINT_LIGHTS] = {
    vec3(4.5f, 4.0f, 3.0f),
    vec3(-4.5f, 4.0f, 3.0f),
    vec3(4.5f, -4.0f, 3.5f),
    vec3(-4.5f, -4.0f, 3.5f)
  };

  vec3 light_colors[N_POINT_LIGHTS] = {
    vec3(1.0f, 0.0f, 0.3f) * 10.0f,
    vec3(0.0f, 1.0f, 0.2f) * 10.0f,
    vec3(0.0f, 0.5f, 1.0f) * 10.0f,
    vec3(1.0f, 0.85f, 0.0f) * 10.0f
  };

  for (int i = 0; i < N_POINT_LIGHTS; i++) {
    Instance light_ball(light_obj, light_program);
    PointLight point_light(2 + i, light_positions[i], light_colors[i]);

    light_ball.transform = translate(mat4(1.0), light_positions[i]);
    light_ball.transform = scale(light_ball.transform, vec3(0.1));

    lights.push_back(light_ball);
    point_lights.push_back(point_light);
  }

  SpotLight flashlight(2 + N_POINT_LIGHTS, camera.position, camera.forward, radians(12.5f), vec3(1.0f, 0.96f, 0.88f));
  flashlight.attenuation = vec3(0.0f, 0.0f, 0.01f);

  // Textures
  Texture container("assets/container2.png", Texture::Type::Diffuse);
  Texture container_spec("assets/container2_spec.png", Texture::Type::Specular);

  // Setup uniforms
  // --------------------------------------------
  cube_program.use();

  cube_program.set("material.diffuse", 0);
  cube_program.set("material.specular", 1);
  cube_program.set("material.shininess", 32.0f);

  dir_light.set_ubo_binding(cube_program, "DirectionalLightBlock");
  dir_light.update_ubo();
  flashlight.set_ubo_binding(cube_program, "SpotLightBlock");
  flashlight.update_ubo();
  for (int i = 0; i < N_POINT_LIGHTS; i++) {
    std::stringstream ss;
    ss << "PointLightBlock" << i;
    std::string str = ss.str();
    point_lights[i].set_ubo_binding(cube_program, str.c_str());
    point_lights[i].update_ubo();
  }

  // Rendering loop
  // --------------------------------------------
  int width, height;
  while (!glfwWindowShouldClose(window)) {
    float current_frame = (float) glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_input(window);

    // Rendering code
    glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glfwGetFramebufferSize(window, &width, &height);
    camera.update_matrices((float) width / (float) height);

    cube_program.use();
    cube_program.set("viewPos", camera.position);

    flashlight.position = camera.position;
    flashlight.direction = camera.forward;
    flashlight.update_ubo();

    container.bind(0);
    container_spec.bind(1);
    for (const auto &light: lights) light.draw();
    for (const auto &cube: cubes) cube.draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
