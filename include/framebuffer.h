#ifndef LEARN_OPENGL_FRAMEBUFFER_H
#define LEARN_OPENGL_FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>

class Framebuffer {
protected:
  unsigned _id;

public:
  Framebuffer();

  unsigned id() const;

  void bind() const;

  static void unbind();

  virtual void free() = 0;
};

class TextureFramebuffer : public Framebuffer {
private:
  std::vector<unsigned> textures;
  unsigned _rbo;

public:
  TextureFramebuffer(int width, int height, GLint internal_format = GL_RGB16F, unsigned num_textures = 1);

  unsigned texture(unsigned idx = 0) const;

  void bind_texture(unsigned texture_unit = 0, unsigned idx = 0) const;

  void free() override;
};

class DepthFramebuffer : public Framebuffer {
private:
  unsigned _depth;

public:
  DepthFramebuffer(int width, int height);

  unsigned depth_map() const;

  void free() override;
};

#endif //LEARN_OPENGL_FRAMEBUFFER_H
