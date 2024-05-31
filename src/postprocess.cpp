#include <postprocess.h>

PostProcessing::PostProcessing(int width, int height, Program &final_stage)
  : final_stage(final_stage), viewport_width(width), viewport_height(height) {
  framebuffers[0] = std::make_unique<TextureFramebuffer>(TextureFramebuffer(width, height));
  framebuffers[1] = std::make_unique<TextureFramebuffer>(TextureFramebuffer(width, height));

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
}

void PostProcessing::resize_framebuffers(int width, int height) {
  framebuffers[0]->free();
  framebuffers[1]->free();

  framebuffers[0] = std::make_unique<TextureFramebuffer>(TextureFramebuffer(width, height));
  framebuffers[1] = std::make_unique<TextureFramebuffer>(TextureFramebuffer(width, height));

  viewport_width = width;
  viewport_height = height;
}

void PostProcessing::bind_input_framebuffer() const {
  framebuffers[0]->bind();
}

void PostProcessing::run() {
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_FRAMEBUFFER_SRGB);

  read_fb = 0;
  for (const auto &stage: stages) {
    stage(read_buffer(), write_buffer(), viewport_width, viewport_height, *screen_quad);
    swap_buffers();
  }

  // Final stage
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  final_stage.use();
  final_stage.set("screenWidth", viewport_width);
  final_stage.set("screenHeight", viewport_height);
  read_buffer().bind_texture();
  screen_quad->draw(final_stage);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_FRAMEBUFFER_SRGB);
}

TextureFramebuffer &PostProcessing::read_buffer() const {
  return *framebuffers[read_fb];
}

TextureFramebuffer &PostProcessing::write_buffer() const {
  return *framebuffers[1 - read_fb];
}

void PostProcessing::swap_buffers() {
  read_fb = 1 - read_fb;
}

void PostProcessing::add_stage(const PostProcessingStage &stage) {
  stages.push_back(stage);
}

PostProcessingStage make_shader_stage(const Program &program) {
  return [program](
    TextureFramebuffer &read_buffer,
    Framebuffer &write_buffer,
    int vw,
    int vh,
    const Mesh &screen_quad
  ) {
    write_buffer.bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    program.use();
    program.set("screenWidth", vw);
    program.set("screenHeight", vh);
    read_buffer.bind_texture(0);
    screen_quad.draw(program);
    Framebuffer::unbind();
  };
}
