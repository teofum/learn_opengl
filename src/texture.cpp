#include <glad/glad.h>
#include <stb/stb_image.h>

#include <iostream>
#include <utility>

#include <texture.h>

Texture::Texture(
  const char *path,
  Type type,
  GLint wrap_mode_s,
  GLint wrap_mode_t,
  GLint min_filter,
  GLint mag_filter,
  bool gen_mipmaps
) : _id(0), path(std::string(path)), _type(type) {
  glGenTextures(1, &_id);

  int width, height, n_channels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path, &width, &height, &n_channels, 0);

  if ((load_status = (data != nullptr))) {
    GLenum format = GL_RGB;
    if (n_channels == 1) format = GL_RED;
    else if (n_channels == 3) format = GL_RGB;
    else if (n_channels == 4) format = GL_RGBA;


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    if (gen_mipmaps) glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
  } else {
    std::cerr << "ERROR::TEXTURE::LOAD_FAILED\n";
  }

  stbi_image_free(data);
}

Texture::Texture(
  const char *path,
  Type type,
  GLint wrap_mode,
  GLint min_filter,
  GLint mag_filter,
  bool gen_mipmaps
)
  : Texture(path, type, wrap_mode, wrap_mode, min_filter, mag_filter, gen_mipmaps) {
}

Texture::Texture(
  const char *path,
  Type type,
  GLint wrap_mode
)
  : Texture(path, type, wrap_mode, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true) {
}

Texture::Texture(const char *path, Type type)
  : Texture(path, type, GL_REPEAT) {
}

unsigned Texture::id() const {
  return _id;
}

Texture::Type Texture::type() const {
  return _type;
}

bool Texture::ready() const {
  return load_status != 0;
}

void Texture::bind(unsigned char texture_unit) const {
  glActiveTexture(GL_TEXTURE0 + texture_unit);
  glBindTexture(GL_TEXTURE_2D, _id);
}

const std::string &Texture::image_path() const {
  return path;
}
