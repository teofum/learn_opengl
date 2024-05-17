#include <framebuffer.h>

TextureFramebuffer::TextureFramebuffer(int width, int height) : _id(0), _texture(0) {
  glGenFramebuffers(1, &_id);
  glBindFramebuffer(GL_FRAMEBUFFER, _id);

  glGenTextures(1, &_texture);
  glBindTexture(GL_TEXTURE_2D, _texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  unsigned rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned TextureFramebuffer::id() const {
  return _id;
}

unsigned TextureFramebuffer::texture() const {
  return _texture;
}

void TextureFramebuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void TextureFramebuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
