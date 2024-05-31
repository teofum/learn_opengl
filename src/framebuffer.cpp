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

TextureFramebuffer::TextureFramebuffer(int width, int height, GLint internal_format, unsigned num_textures)
  : Framebuffer(), _rbo(0) {
  glBindFramebuffer(GL_FRAMEBUFFER, _id);

  textures.resize(num_textures);
  glGenTextures((int) num_textures, textures.data());

  std::vector<unsigned> attachments;
  for (int i = 0; i < num_textures; i++) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned attachment = GL_COLOR_ATTACHMENT0 + i;
    attachments.push_back(attachment);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textures[i], 0);
  }
  glDrawBuffers((int) num_textures, attachments.data());

  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned TextureFramebuffer::texture(unsigned idx) const {
  return textures[idx];
}

void TextureFramebuffer::free() {
  glDeleteTextures((int) textures.size(), textures.data());
  glDeleteRenderbuffers(1, &_rbo);
  glDeleteFramebuffers(1, &_id);
}

void TextureFramebuffer::bind_texture(unsigned texture_unit, unsigned idx) const {
  glActiveTexture(GL_TEXTURE0 + texture_unit);
  glBindTexture(GL_TEXTURE_2D, textures[idx]);
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

void DepthFramebuffer::free() {
  glDeleteTextures(1, &_depth);
  glDeleteFramebuffers(1, &_id);
}
