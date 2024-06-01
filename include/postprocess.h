#ifndef LEARN_OPENGL_POSTPROCESS_H
#define LEARN_OPENGL_POSTPROCESS_H

#include <functional>
#include <memory>
#include <vector>

#include <framebuffer.h>
#include <mesh.h>
#include <program.h>

using PostProcessingStage = std::function<void(TextureFramebuffer &, TextureFramebuffer &, int, int, const Mesh &)>;

PostProcessingStage make_shader_stage(const Program &program);

class PostProcessing {
private:
  std::unique_ptr<Mesh> screen_quad;
  std::unique_ptr<TextureFramebuffer> framebuffers[2];
  std::vector<PostProcessingStage> stages;
  unsigned read_fb = 0;
  int viewport_width, viewport_height;

  TextureFramebuffer &read_buffer() const;

  TextureFramebuffer &write_buffer() const;

  void swap_buffers();

public:
  Program &final_stage;

  PostProcessing(int width, int height, Program &final_stage);

  void resize_framebuffers(int width, int height);

  unsigned input_framebuffer() const;

  void bind_input_framebuffer() const;

  void run();

  void add_stage(const PostProcessingStage &stage);
};

#endif // LEARN_OPENGL_POSTPROCESS_H
