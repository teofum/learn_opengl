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
#include <random>

#define WIDTH 800
#define HEIGHT 600

#define N_LIGHTS 10

using namespace glm;

struct LightData {
  Instance obj;
  PointLight light;
  DepthCubeFramebuffer shadow_buffer;
  vec3 rotation_axis;
  float rotation_speed, radius;
};

class DeferredRenderingWindow : public Window {
public:
  DeferredRenderingWindow() : Window(WIDTH, HEIGHT, "Learn OpenGL 10 — Deferred rendering") {
  }

private:
  Program g_program, light_program, shadow_program, tonemap_program, deferred_program;

  std::unique_ptr<Model> room_model, box_model, light_model;
  std::unique_ptr<Instance> room;
  std::vector<Instance> boxes;

  std::vector<LightData> lights;

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

    deferred_program = Program("shaders/deferred-rendering/d_vert.glsl", "shaders/deferred-rendering/d_frag.glsl");

    camera->set_matrix_binding(g_program);
    camera->set_matrix_binding(light_program);
    camera->set_matrix_binding(deferred_program);

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

    // Setup lights
    // --------------------------------------------

    light_model = std::make_unique<Model>(Model("assets/sphere.obj"));

    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_real_distribution<float> col(0.1f, 1.0f);
    std::uniform_real_distribution<float> pos(-1.0f, 1.0f);

    for (unsigned i = 0; i < N_LIGHTS; i++) {
      vec3 light_color(col(e1), col(e1), col(e1));
      vec3 rotation_axis(pos(e1), pos(e1), pos(e1));
      float rotation_speed = (0.5f + col(e1)) * 45.0f;

      PointLight light(1 + i, vec3(0.0f), light_color * 10.0f);
      Instance light_obj(*light_model, light_program);
      DepthCubeFramebuffer shadow_buffer(SHADOW_WIDTH, SHADOW_HEIGHT);

      float threshold = 5.0f / 256.0f;
      float i_max = max(light.diffuse.x, max(light.diffuse.y, light.diffuse.z));
      float a = light.attenuation.z, b = light.attenuation.y;
      float c = light.attenuation.x - i_max / threshold;
      float radius = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

      lights.push_back(
        {
          light_obj,
          light,
          shadow_buffer,
          normalize(rotation_axis),
          rotation_speed,
          radius
        }
      );
    }

    // Setup uniforms
    // --------------------------------------------
    deferred_program.use();
    deferred_program.set("gPosition", 0);
    deferred_program.set("gNormal", 1);
    deferred_program.set("gAlbedoSpec", 2);

    deferred_program.set("shadowMap", 10);
    deferred_program.set("farPlane", POINT_SHADOW_FAR);

    shadow_program.use();
    shadow_program.set("farPlane", POINT_SHADOW_FAR);

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
    for (auto &light: lights) {
      light.obj.transform = rotate(mat4(1.0), radians(current_frame * light.rotation_speed), light.rotation_axis);
      light.obj.transform = translate(light.obj.transform, vec3(0.0, 0.0, 4.0));

      light.light.position = vec3(light.obj.transform * vec4(0.0, 0.0, 0.0, 1.0));
      light.light.update_ubo();
    }

    // Render shadow depth maps
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    for (auto &light: lights) {
      light.shadow_buffer.bind();
      glClear(GL_DEPTH_BUFFER_BIT);
      light.light.set_ubo_binding(shadow_program, "PointLightBlock");
      for (const auto &box: boxes) box.draw_with(shadow_program);
    }
    Framebuffer::unbind();

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

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glCullFace(GL_FRONT);
    for (auto &light: lights) {
      light.light.set_ubo_binding(deferred_program, "PointLightBlock");
      glActiveTexture(GL_TEXTURE10);
      glBindTexture(GL_TEXTURE_CUBE_MAP, light.shadow_buffer.depth_map());

      light.obj.transform = scale(light.obj.transform, vec3(light.radius));
      light.obj.draw_with(deferred_program);
    }
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Forward rendering pass (lights)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer->id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, post_processing->input_framebuffer());
    glBlitFramebuffer(
      0, 0, viewport_width, viewport_height,
      0, 0, viewport_width, viewport_height,
      GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    post_processing->bind_input_framebuffer();

    for (auto &light: lights) {
      light.obj.transform = scale(light.obj.transform, vec3(0.05f / light.radius));
      light.obj.draw();
    }

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
