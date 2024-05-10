#include <glad/glad.h>
#include <stb/stb_image.h>

#include <iostream>

#include <texture.h>

Texture::Texture(
  const char *img_path,
  GLenum format,
  GLint wrap_mode_s,
  GLint wrap_mode_t,
  GLint min_filter,
  GLint mag_filter,
  bool gen_mipmaps
) : _id(0) {
  glGenTextures(1, &_id);
  glBindTexture(GL_TEXTURE_2D, _id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode_s);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode_t);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

  int width, height, n_channels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(img_path, &width, &height, &n_channels, 0);

  if ((load_status = (data != nullptr))) {
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    if (gen_mipmaps) glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << "ERROR::TEXTURE::LOAD_FAILED\n";
  }

  stbi_image_free(data);
}

Texture::Texture(
  const char *img_path,
  GLenum format,
  GLint wrap_mode,
  GLint min_filter,
  GLint mag_filter,
  bool gen_mipmaps
)
  : Texture(img_path, format, wrap_mode, wrap_mode, min_filter, mag_filter, gen_mipmaps) {
}

Texture::Texture(const char *img_path, GLenum format, GLint wrap_mode)
  : Texture(img_path, format, wrap_mode, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true) {

}

Texture::Texture(const char *img_path, GLenum format)
  : Texture(img_path, format, GL_REPEAT) {
}

Texture::~Texture() {
  glDeleteTextures(1, &_id);
}

unsigned Texture::id() const {
  return _id;
}

bool Texture::ready() const {
  return load_status != 0;
}

void Texture::bind(unsigned char texture_unit) const {
  glActiveTexture(GL_TEXTURE0 + texture_unit);
  glBindTexture(GL_TEXTURE_2D, _id);
}
