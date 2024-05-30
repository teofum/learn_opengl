#include <framebuffer.h>

Framebuffer::Framebuffer() : _id(0) {
  glGenFramebuffers(1, &_id);
}

unsigned Framebuffer::id() const {
  return _id;
}

void Framebuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void Framebuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

TextureFramebuffer::TextureFramebuffer(int width, int height, GLint internal_format)
  : Framebuffer(), _texture(0), _rbo(0) {
  glBindFramebuffer(GL_FRAMEBUFFER, _id);

  glGenTextures(1, &_texture);
  glBindTexture(GL_TEXTURE_2D, _texture);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned TextureFramebuffer::texture() const {
  return _texture;
}

void TextureFramebuffer::free() {
  glDeleteTextures(1, &_texture);
  glDeleteRenderbuffers(1, &_rbo);
  glDeleteFramebuffers(1, &_id);
}

DepthFramebuffer::DepthFramebuffer(int width, int height) : Framebuffer(), _depth(0) {
  glBindFramebuffer(GL_FRAMEBUFFER, _id);

  glGenTextures(1, &_depth);
  glBindTexture(GL_TEXTURE_2D, _depth);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned DepthFramebuffer::depth_map() const {
  return _depth;
}
