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
  GLFWwindow *window = init_window(WIDTH, HEIGHT, "Learn OpenGL 09 — Point shadows");
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
  Shader vertex_shader = Shader::vertex("shaders/point-shadow/vertex.glsl");
  Shader basic_frag = Shader::fragment("shaders/point-shadow/fragment.glsl");
  Shader light_frag = Shader::fragment("shaders/point-shadow/light_frag.glsl");

  Program program(vertex_shader, basic_frag);
  Program light_program(vertex_shader, light_frag);
  Program shadow_program;
  shadow_program.attach_shader(Shader::vertex("shaders/point-shadow/shadow_vert.glsl"));
  shadow_program.attach_shader(Shader::geometry("shaders/point-shadow/shadow_geom.glsl"));
  shadow_program.attach_shader(Shader::fragment("shaders/point-shadow/shadow_frag.glsl"));
  shadow_program.link();

  camera.set_matrix_binding(program);
  camera.set_matrix_binding(light_program);

  // Setup objects
  // --------------------------------------------
  Model room_model("assets/brick_container.obj", true);
  Instance room(room_model, program);

  room.transform = translate(room.transform, vec3(0.0f, -5.0f, 0.0f));
  room.transform = scale(room.transform, vec3(10.0f));

  Model box_model("assets/container.obj");
  Instance box1(box_model, program);
  Instance box2(box_model, program);
  Instance box3(box_model, program);
  Instance box4(box_model, program);
  Instance box5(box_model, program);

  box1.transform = translate(box1.transform, vec3(3.0f, -2.0f, 2.0f));
  box3.transform = translate(box3.transform, vec3(-3.0f, 2.0f, 2.0f));
  box4.transform = translate(box4.transform, vec3(3.0f, 2.0f, -2.0f));
  box5.transform = translate(box5.transform, vec3(-3.0f, -2.0f, -2.0f));

  box2.transform = translate(box2.transform, vec3(-1.0f, 1.0f, -1.0f));
  box2.transform = rotate(box2.transform, radians(55.0f), vec3(-1.0f, 1.0f, -1.0f));

  Model light_model("assets/sphere.obj");
  Instance light_obj0(light_model, light_program);
  Instance light_obj1(light_model, light_program);

  // Lights
  PointLight light0(1, vec3(0.0f), vec3(1.0f, 0.5f, 0.2f) * 10.0f);
  light0.set_ubo_binding(program, "PointLightBlock0");

  PointLight light1(2, vec3(0.0f), vec3(0.2f, 0.3f, 1.0f) * 10.0f);
  light1.set_ubo_binding(program, "PointLightBlock1");

  // Setup uniforms
  // --------------------------------------------
  program.use();
  program.set("shadowMap0", 10);
  program.set("shadowMap1", 11);
  program.set("farPlane", POINT_SHADOW_FAR);
  program.set("material.shininess", 32.0f);

  shadow_program.use();
  shadow_program.set("farPlane", POINT_SHADOW_FAR);

  // Setup shadow framebuffer
  // --------------------------------------------
  unsigned depth_cubemap0;
  glGenTextures(1, &depth_cubemap0);

  glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap0);
  for (unsigned i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; i++) {
    glTexImage2D(i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  unsigned shadow_depth_fbo0;
  glGenFramebuffers(1, &shadow_depth_fbo0);
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fbo0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap0, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  unsigned depth_cubemap1;
  glGenTextures(1, &depth_cubemap1);

  glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap1);
  for (unsigned i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; i++) {
    glTexImage2D(i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  unsigned shadow_depth_fbo1;
  glGenFramebuffers(1, &shadow_depth_fbo1);
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fbo1);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap1, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


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

    // Update lights
    light0.position = vec3(cos(current_frame) * 2.0f, 0.0f, sin(current_frame) * 2.0f);
    light0.update_ubo();
    light_obj0.transform = translate(mat4(1.0), light0.position);
    light_obj0.transform = scale(light_obj0.transform, vec3(0.05f));

    light1.position = vec3(sin(current_frame * 0.5f) * 2.0f, cos(current_frame * 0.5f) * 2.0f, 0.0f);
    light1.update_ubo();
    light_obj1.transform = translate(mat4(1.0), light1.position);
    light_obj1.transform = scale(light_obj1.transform, vec3(0.05f));

    // Render shadow depth maps
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    light0.set_ubo_binding(shadow_program, "PointLightBlock");
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fbo0);
    glClear(GL_DEPTH_BUFFER_BIT);
    box1.draw_with(shadow_program);
    box2.draw_with(shadow_program);
    box3.draw_with(shadow_program);
    box4.draw_with(shadow_program);
    box5.draw_with(shadow_program);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    light1.set_ubo_binding(shadow_program, "PointLightBlock");
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fbo1);
    glClear(GL_DEPTH_BUFFER_BIT);
    box1.draw_with(shadow_program);
    box2.draw_with(shadow_program);
    box3.draw_with(shadow_program);
    box4.draw_with(shadow_program);
    box5.draw_with(shadow_program);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Rendering code
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.update_matrices((float) width / (float) height);

    program.use();
    program.set("viewPos", camera.position);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap0);
    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap1);

    box1.draw();
    box2.draw();
    box3.draw();
    box4.draw();
    box5.draw();

    light_obj0.draw();
    light_obj1.draw();

    glCullFace(GL_FRONT);
    room.draw();
    glCullFace(GL_BACK);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
