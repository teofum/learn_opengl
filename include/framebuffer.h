#ifndef LEARN_OPENGL_FRAMEBUFFER_H
#define LEARN_OPENGL_FRAMEBUFFER_H

#include <glad/glad.h>

class TextureFramebuffer {
private:
  unsigned _id;
  unsigned _texture;

public:
  TextureFramebuffer(int width, int height);

  unsigned id() const;

  unsigned texture() const;

  void bind() const;

  static void unbind();
};

#endif //LEARN_OPENGL_FRAMEBUFFER_H
