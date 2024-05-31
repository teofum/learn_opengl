#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <program.h>
#include <model.h>
#include <instance.h>
#include <window.h>
#include <light.h>

#define WIDTH 800
#define HEIGHT 600

using namespace glm;

class PointShadowWindow : public Window {
public:
  PointShadowWindow() : Window(WIDTH, HEIGHT, "Learn OpenGL 09 — Point shadows") {
  }

private:
  Program program, light_program, shadow_program, bloom_program, blur_program_h, blur_program_v, add_program;
  std::vector<Program> post_programs;

  std::unique_ptr<Model> room_model, box_model, light_model;
  std::unique_ptr<Instance> room;
  std::vector<Instance> boxes;
  std::vector<Instance> light_objs;

  std::vector<PointLight> lights;
  std::vector<unsigned> depth_cubemaps;
  std::vector<unsigned> shadow_depth_fbos;

  std::unique_ptr<TextureFramebuffer> render_framebuffer;
  std::unique_ptr<TextureFramebuffer> bloom_framebuffer;
  std::unique_ptr<TextureFramebuffer> bloom_framebuffer_2;
  std::unique_ptr<Mesh> screen_quad;

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

    Shader pp_vertex = Shader::vertex("shaders/point-shadow/pp_vert.glsl");
    Shader pp_tonemap[3] = {
      Shader::fragment("shaders/point-shadow/pp_frag_tm_none.glsl"),
      Shader::fragment("shaders/point-shadow/pp_frag_tm_reinhard.glsl"),
      Shader::fragment("shaders/point-shadow/pp_frag_tm_aces.glsl")
    };
    for (const auto &tm_frag: pp_tonemap) {
      post_programs.emplace_back(pp_vertex, tm_frag);
    }

    Shader pp_bloom = Shader::fragment("shaders/point-shadow/pp_frag_bloom.glsl");
    bloom_program = Program(pp_vertex, pp_bloom);

    Shader pp_blur_h = Shader::fragment("shaders/point-shadow/pp_frag_blur_h.glsl");
    Shader pp_blur_v = Shader::fragment("shaders/point-shadow/pp_frag_blur_v.glsl");
    blur_program_h = Program(pp_vertex, pp_blur_h);
    blur_program_v = Program(pp_vertex, pp_blur_v);

    Shader pp_add = Shader::fragment("shaders/point-shadow/pp_frag_bloom_add.glsl");
    add_program = Program(pp_vertex, pp_add);

    camera->set_matrix_binding(program);
    camera->set_matrix_binding(light_program);

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

    add_program.use();
    add_program.set("screenTexture", 0);
    add_program.set("bloomTexture", 1);

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

    // Setup framebuffers
    // --------------------------------------------
    render_framebuffer = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(viewport_width, viewport_height)
    );
    bloom_framebuffer = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(viewport_width, viewport_height)
    );
    bloom_framebuffer_2 = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(viewport_width, viewport_height)
    );

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

    render_framebuffer->free();
    render_framebuffer = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(width, height)
    );

    bloom_framebuffer->free();
    bloom_framebuffer = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(width, height)
    );

    bloom_framebuffer_2->free();
    bloom_framebuffer_2 = std::make_unique<TextureFramebuffer>(
      TextureFramebuffer(width, height)
    );
  }

  void key_callback(int key, int scancode, int action, int mods) override {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_SPACE)
      pp_frag_idx = (pp_frag_idx + 1) % post_programs.size();
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

    // Rendering code
    glViewport(0, 0, viewport_width, viewport_height);
    camera->update_matrices(aspect_ratio());

    program.use();
    program.set("viewPos", camera->position);

    render_framebuffer->bind();
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
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    // Bloom
    bloom_framebuffer->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    bloom_program.use();
    bloom_program.set("screenWidth", viewport_width);
    bloom_program.set("screenHeight", viewport_height);
    render_framebuffer->bind_texture();
    screen_quad->draw(bloom_program);
    Framebuffer::unbind();

    bloom_framebuffer_2->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    blur_program_h.use();
    blur_program_h.set("screenWidth", viewport_width);
    blur_program_h.set("screenHeight", viewport_height);
    bloom_framebuffer->bind_texture();
    screen_quad->draw(blur_program_h);
    Framebuffer::unbind();

    bloom_framebuffer->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    blur_program_v.use();
    blur_program_v.set("screenWidth", viewport_width);
    blur_program_v.set("screenHeight", viewport_height);
    bloom_framebuffer_2->bind_texture();
    screen_quad->draw(blur_program_v);
    Framebuffer::unbind();

    bloom_framebuffer_2->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    add_program.use();
    add_program.set("screenWidth", viewport_width);
    add_program.set("screenHeight", viewport_height);
    render_framebuffer->bind_texture(0);
    bloom_framebuffer->bind_texture(1);
    screen_quad->draw(add_program);
    Framebuffer::unbind();

    // Tonemapping
    const auto &post_program = post_programs[pp_frag_idx];
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    post_program.use();
    post_program.set("screenWidth", viewport_width);
    post_program.set("screenHeight", viewport_height);
    bloom_framebuffer_2->bind_texture();
    screen_quad->draw(post_program);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
  }
};

int main() {
  PointShadowWindow window;
  window.start();

  glfwTerminate();
  return 0;
}
