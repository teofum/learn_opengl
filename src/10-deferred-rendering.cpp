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

class DeferredRenderingWindow : public Window {
public:
  DeferredRenderingWindow() : Window(WIDTH, HEIGHT, "Learn OpenGL 10 — Deferred rendering") {
  }

private:
  Program g_program, light_program, shadow_program, tonemap_program, deferred_program;

  std::unique_ptr<Model> room_model, box_model, light_model;
  std::unique_ptr<Instance> room;
  std::vector<Instance> boxes;
  std::vector<Instance> light_objs;

  std::vector<PointLight> lights;
  std::vector<unsigned> depth_cubemaps;
  std::vector<unsigned> shadow_depth_fbos;

  std::unique_ptr<TextureFramebuffer> g_buffer;
  std::unique_ptr<Mesh> screen_quad;
  std::unique_ptr<PostProcessing> post_processing;

  void setup() override {
    camera->position = vec3(0.0, 1.5, 5.0);

    // Compile shaders and link program
    // --------------------------------------------
    Shader vertex_shader = Shader::vertex("shaders/deferred-rendering/g_vert.glsl");
    Shader g_frag = Shader::fragment("shaders/deferred-rendering/g_frag.glsl");
    g_program = Program(vertex_shader, g_frag);

    Shader light_frag = Shader::fragment("shaders/deferred-rendering/light_frag.glsl");
    light_program = Program(vertex_shader, light_frag);

    shadow_program = Program();
    shadow_program.attach_shader(Shader::vertex("shaders/point-shadow/shadow_vert.glsl"));
    shadow_program.attach_shader(Shader::geometry("shaders/point-shadow/shadow_geom.glsl"));
    shadow_program.attach_shader(Shader::fragment("shaders/point-shadow/shadow_frag.glsl"));
    shadow_program.link();

    tonemap_program = Program("shaders/common/postprocess/vert.glsl", "shaders/common/postprocess/frag_tm_aces.glsl");

    deferred_program = Program("shaders/common/postprocess/vert.glsl", "shaders/deferred-rendering/d_frag.glsl");

    camera->set_matrix_binding(g_program);
    camera->set_matrix_binding(light_program);

    // Setup G_Buffer
    // --------------------------------------------
    g_buffer = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(viewport_width, viewport_height, std::vector<GLint>{GL_RGBA32F, GL_RGBA32F, GL_RGBA})
    );

    // Setup post processing
    // --------------------------------------------
    post_processing = std::make_unique<PostProcessing>(
      PostProcessing(viewport_width, viewport_height, tonemap_program)
    );

    PostProcessBloom bloom(viewport_width, viewport_height, 5);
    post_processing->add_stage(bloom);

    // Setup objects
    // --------------------------------------------
    room_model = std::make_unique<Model>(Model("assets/brick_container.obj", true));
    room = std::make_unique<Instance>(Instance(*room_model, g_program));

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
      Instance box(*box_model, g_program);
      box.transform = transform;
      boxes.push_back(box);
    }

    light_model = std::make_unique<Model>(Model("assets/sphere.obj"));
    light_objs.emplace_back(*light_model, light_program);
    light_objs.emplace_back(*light_model, light_program);

    // Lights
    PointLight light0(1, vec3(0.0f), vec3(1.0f, 0.5f, 0.0f) * 100.0f);
    light0.set_ubo_binding(deferred_program, "PointLightBlock0");
    lights.push_back(light0);

    PointLight light1(2, vec3(0.0f), vec3(0.0f, 0.3f, 1.0f) * 100.0f);
    light1.set_ubo_binding(deferred_program, "PointLightBlock1");
    lights.push_back(light1);

    // Setup uniforms
    // --------------------------------------------
    deferred_program.use();
    deferred_program.set("gPosition", 0);
    deferred_program.set("gNormal", 1);
    deferred_program.set("gAlbedoSpec", 2);

    deferred_program.set("shadowMap0", 10);
    deferred_program.set("shadowMap1", 11);
    deferred_program.set("farPlane", POINT_SHADOW_FAR);
    deferred_program.set("material.shininess", 32.0f);

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
    screen_quad = std::make_unique<Mesh>(
      Mesh(std::move(quad_vertices), std::move(quad_indices), std::move(quad_textures))
    );

    glDepthFunc(GL_LEQUAL);
  }

  void resize_callback(int width, int height) override {
    Window::resize_callback(width, height);

    post_processing->resize_framebuffers(width, height);

    g_buffer->free();
    g_buffer = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(width, height, std::vector<GLint>{GL_RGBA32F, GL_RGBA32F, GL_RGBA})
    );
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

    // Geometry pass
    glViewport(0, 0, viewport_width, viewport_height);
    camera->update_matrices(aspect_ratio());

    g_buffer->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (const auto &box: boxes) box.draw();
    glCullFace(GL_FRONT);
    room->draw();
    glCullFace(GL_BACK);
    Framebuffer::unbind();

    // Deferred lighting pass
    post_processing->bind_input_framebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    deferred_program.use();
    deferred_program.set("viewPos", camera->position);
    g_buffer->bind_texture(0, 0);
    g_buffer->bind_texture(1, 1);
    g_buffer->bind_texture(2, 2);

    for (int i = 0; i < 2; i++) {
      glActiveTexture(GL_TEXTURE10 + i);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemaps[i]);
    }

    screen_quad->draw(deferred_program);

    // Forward rendering pass (lights)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer->id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, post_processing->input_framebuffer());
    glBlitFramebuffer(
      0, 0, viewport_width, viewport_height,
      0, 0, viewport_width, viewport_height,
      GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    post_processing->bind_input_framebuffer();

    for (const auto &light: light_objs) light.draw();

    Framebuffer::unbind();

    // Postprocessing
    post_processing->run();
  }
};

int main() {
  DeferredRenderingWindow window;
  window.start();

  glfwTerminate();
  return 0;
}
