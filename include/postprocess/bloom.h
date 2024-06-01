#ifndef LEARN_OPENGL_BLOOM_H
#define LEARN_OPENGL_BLOOM_H

#include <framebuffer.h>
#include <mesh.h>
#include <program.h>

class PostProcessBloom {
private:
  Program bloom_program, blur_program_h, blur_program_v, add_program;
  TextureFramebuffer internal_buffer;
  int width, height;
  unsigned iterations;

public:
  PostProcessBloom(int width, int height, unsigned iterations = 1);

  void operator()(
    TextureFramebuffer &read_buffer,
    TextureFramebuffer &write_buffer,
    int viewport_width,
    int viewport_height,
    const Mesh &screen_quad
  );
};

#endif //LEARN_OPENGL_BLOOM_H
