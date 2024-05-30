#ifndef LEARN_OPENGL_FRAMEBUFFER_H
#define LEARN_OPENGL_FRAMEBUFFER_H

#include <glad/glad.h>

class Framebuffer {
protected:
  unsigned _id;

public:
  Framebuffer();

  unsigned id() const;

  void bind() const;

  static void unbind();
};

class TextureFramebuffer : public Framebuffer {
private:
  unsigned _texture;

public:
  TextureFramebuffer(int width, int height, GLint internal_format = GL_RGB16F);

  unsigned texture() const;
};

class DepthFramebuffer : public Framebuffer {
private:
  unsigned _depth;

public:
  DepthFramebuffer(int width, int height);

  unsigned depth_map() const;
};

#endif //LEARN_OPENGL_FRAMEBUFFER_H
