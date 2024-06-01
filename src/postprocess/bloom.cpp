#include <postprocess/bloom.h>

PostProcessBloom::PostProcessBloom(int width, int height, unsigned iterations)
  : width(width), height(height), iterations(iterations), internal_buffer(width, height) {
  Shader pp_vertex = Shader::vertex("shaders/common/postprocess/vert.glsl");

  Shader pp_bloom = Shader::fragment("shaders/common/postprocess/frag_bloom.glsl");
  bloom_program = Program(pp_vertex, pp_bloom);

  Shader pp_blur_h = Shader::fragment("shaders/common/postprocess/frag_blur_h.glsl");
  Shader pp_blur_v = Shader::fragment("shaders/common/postprocess/frag_blur_v.glsl");
  blur_program_h = Program(pp_vertex, pp_blur_h);
  blur_program_v = Program(pp_vertex, pp_blur_v);

  Shader pp_add = Shader::fragment("shaders/common/postprocess/frag_bloom_add.glsl");
  add_program = Program(pp_vertex, pp_add);

  add_program.use();
  add_program.set("screenTexture", 0);
  add_program.set("bloomTexture", 1);
}

void PostProcessBloom::operator()(
  TextureFramebuffer &read_buffer,
  TextureFramebuffer &write_buffer,
  int viewport_width,
  int viewport_height,
  const Mesh &screen_quad
) {
  if (width != viewport_width || height != viewport_height) {
    width = viewport_width;
    height = viewport_height;

    internal_buffer.free();
    internal_buffer = TextureFramebuffer(width, height);
  }

  internal_buffer.bind();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  bloom_program.use();
  bloom_program.set("screenWidth", viewport_width);
  bloom_program.set("screenHeight", viewport_height);
  read_buffer.bind_texture();
  screen_quad.draw(bloom_program);

  for (int i = 0; i < iterations; i++) {
    write_buffer.bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    blur_program_h.use();
    blur_program_h.set("screenWidth", viewport_width);
    blur_program_h.set("screenHeight", viewport_height);
    internal_buffer.bind_texture();
    screen_quad.draw(blur_program_h);

    internal_buffer.bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    blur_program_v.use();
    blur_program_v.set("screenWidth", viewport_width);
    blur_program_v.set("screenHeight", viewport_height);
    write_buffer.bind_texture();
    screen_quad.draw(blur_program_v);
  }

  write_buffer.bind();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  add_program.use();
  add_program.set("screenWidth", viewport_width);
  add_program.set("screenHeight", viewport_height);
  read_buffer.bind_texture(0);
  internal_buffer.bind_texture(1);
  screen_quad.draw(add_program);
  Framebuffer::unbind();
}
