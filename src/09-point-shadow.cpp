#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <program.h>
#include <model.h>
#include <instance.h>
#include <window.h>
#include <light.h>
#include <postprocess.h>
#include <postprocess/bloom.h>

#define WIDTH 800
#define HEIGHT 600

using namespace glm;

class PointShadowWindow : public Window {
public:
  PointShadowWindow() : Window(WIDTH, HEIGHT, "Learn OpenGL 09 — Point shadows") {
  }

private:
  Program program, light_program, shadow_program;
  std::vector<Program> post_programs;

  std::unique_ptr<Model> room_model, box_model, light_model;
  std::unique_ptr<Instance> room;
  std::vector<Instance> boxes;
  std::vector<Instance> light_objs;

  std::vector<PointLight> lights;
  std::vector<unsigned> depth_cubemaps;
  std::vector<unsigned> shadow_depth_fbos;

  std::unique_ptr<PostProcessing> post_processing;

  unsigned pp_frag_idx = 0;

  void setup() override {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    camera->position = vec3(0.0, 1.5, 5.0);

    // Compile shaders and link cube_program
    // --------------------------------------------
    Shader vertex_shader = Shader::vertex("shaders/point-shadow/vertex.glsl");
    Shader basic_frag = Shader::fragment("shaders/point-shadow/fragment.glsl");
    Shader light_frag = Shader::fragment("shaders/point-shadow/light_frag.glsl");

    program = Program(vertex_shader, basic_frag);
    light_program = Program(vertex_shader, light_frag);
    shadow_program = Program();
    shadow_program.attach_shader(Shader::vertex("shaders/point-shadow/shadow_vert.glsl"));
    shadow_program.attach_shader(Shader::geometry("shaders/point-shadow/shadow_geom.glsl"));
    shadow_program.attach_shader(Shader::fragment("shaders/point-shadow/shadow_frag.glsl"));
    shadow_program.link();

    Shader pp_vertex = Shader::vertex("shaders/common/postprocess/vert.glsl");
    Shader pp_tonemap[3] = {
      Shader::fragment("shaders/common/postprocess/frag_tm_none.glsl"),
      Shader::fragment("shaders/common/postprocess/frag_tm_reinhard.glsl"),
      Shader::fragment("shaders/common/postprocess/frag_tm_aces.glsl")
    };
    for (const auto &tm_frag: pp_tonemap) {
      post_programs.emplace_back(pp_vertex, tm_frag);
    }

    camera->set_matrix_binding(program);
    camera->set_matrix_binding(light_program);

    // Setup post processing
    // --------------------------------------------
    post_processing = std::make_unique<PostProcessing>(
      PostProcessing(viewport_width, viewport_height, post_programs[2])
    );

    PostProcessBloom bloom(viewport_width, viewport_height, 5);
    post_processing->add_stage(bloom);

    // Setup objects
    // --------------------------------------------
    room_model = std::make_unique<Model>(Model("assets/brick_container.obj", true));
    room = std::make_unique<Instance>(Instance(*room_model, program));

    room->transform = translate(room->transform, vec3(0.0f, -5.0f, 0.0f));
    room->transform = scale(room->transform, vec3(10.0f));

    box_model = std::make_unique<Model>(Model("assets/container.obj"));
    mat4 box_transforms[6] = {
      translate(mat4(1.0f), vec3(3.0f, -2.0f, 2.0f)),
      translate(mat4(1.0f), vec3(-3.0f, 2.0f, 2.0f)),
      translate(mat4(1.0f), vec3(3.0f, 2.0f, -2.0f)),
      translate(mat4(1.0f), vec3(-3.0f, -2.0f, -2.0f)),
      rotate(
        translate(mat4(1.0f), vec3(-1.0f, 1.0f, -1.0f)),
        radians(55.0f), vec3(-1.0f, 1.0f, -1.0f)
      )
    };

    for (const auto &transform: box_transforms) {
      Instance box(*box_model, program);
      box.transform = transform;
      boxes.push_back(box);
    }

    light_model = std::make_unique<Model>(Model("assets/sphere.obj"));
    light_objs.emplace_back(*light_model, light_program);
    light_objs.emplace_back(*light_model, light_program);

    // Lights
    PointLight light0(1, vec3(0.0f), vec3(1.0f, 0.5f, 0.0f) * 100.0f);
    light0.set_ubo_binding(program, "PointLightBlock0");
    lights.push_back(light0);

    PointLight light1(2, vec3(0.0f), vec3(0.0f, 0.3f, 1.0f) * 100.0f);
    light1.set_ubo_binding(program, "PointLightBlock1");
    lights.push_back(light1);

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
    unsigned depth_cubemap, shadow_depth_fbo;
    for (unsigned i = 0; i < 2; i++) {
      glGenTextures(1, &depth_cubemap);

      glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
      for (unsigned f = GL_TEXTURE_CUBE_MAP_POSITIVE_X; f <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; f++) {
        glTexImage2D(f, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
      }

      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      glGenFramebuffers(1, &shadow_depth_fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fbo);
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      depth_cubemaps.push_back(depth_cubemap);
      shadow_depth_fbos.push_back(shadow_depth_fbo);
    }

    glDepthFunc(GL_LEQUAL);
  }

  void resize_callback(int width, int height) override {
    Window::resize_callback(width, height);

    post_processing->resize_framebuffers(width, height);
  }

  void key_callback(int key, int scancode, int action, int mods) override {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_SPACE) {
      pp_frag_idx = (pp_frag_idx + 1) % post_programs.size();
      post_processing->final_stage = post_programs[pp_frag_idx];
    }
  }

  void frame() override {
    // Update lights
    lights[0].position = vec3(cos(current_frame) * 4.0f, 0.0f, sin(current_frame) * 4.0f);
    lights[0].update_ubo();
    light_objs[0].transform = translate(mat4(1.0), lights[0].position);
    light_objs[0].transform = scale(light_objs[0].transform, vec3(0.05f));

    lights[1].position = vec3(sin(current_frame * 0.5f) * 4.0f, cos(current_frame * 0.5f) * 4.0f, 0.0f);
    lights[1].update_ubo();
    light_objs[1].transform = translate(mat4(1.0), lights[1].position);
    light_objs[1].transform = scale(light_objs[1].transform, vec3(0.05f));

    // Render shadow depth maps
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    for (int i = 0; i < 2; i++) {
      lights[i].set_ubo_binding(shadow_program, "PointLightBlock");
      glBindFramebuffer(GL_FRAMEBUFFER, shadow_depth_fbos[i]);
      glClear(GL_DEPTH_BUFFER_BIT);
      for (const auto &box: boxes) box.draw_with(shadow_program);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Forward rendering pass
    glViewport(0, 0, viewport_width, viewport_height);
    camera->update_matrices(aspect_ratio());

    program.use();
    program.set("viewPos", camera->position);

    post_processing->bind_input_framebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < 2; i++) {
      glActiveTexture(GL_TEXTURE10 + i);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemaps[i]);
    }

    for (const auto &box: boxes) box.draw();
    for (const auto &light: light_objs) light.draw();

    glCullFace(GL_FRONT);
    room->draw();
    glCullFace(GL_BACK);
    Framebuffer::unbind();

    // Postprocessing
    post_processing->run();
  }
};

int main() {
  PointShadowWindow window;
  window.start();

  glfwTerminate();
  return 0;
}
