#include <glad/glad.h>
#include <stb/stb_image.h>

#include <iostream>

#include <texture.h>

Texture::Texture(
  const char *path,
  Type type,
  GLint wrap_mode_s,
  GLint wrap_mode_t,
  GLint min_filter,
  GLint mag_filter,
  bool gen_mipmaps
) : _id(0), path(std::string(path)), _type(type), _gl_type(GL_TEXTURE_2D) {
  glGenTextures(1, &_id);

  int width, height, n_channels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path, &width, &height, &n_channels, 4);

  if ((load_status = (data != nullptr))) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(_gl_type, _id);

    glTexImage2D(
      _gl_type,
      0,
      (type == Type::Diffuse ? GL_SRGB_ALPHA : GL_RGBA),
      width,
      height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      data
    );

    set_texture_params(wrap_mode_s, wrap_mode_t, 0, min_filter, mag_filter, gen_mipmaps);
  } else {
    std::cerr << "ERROR::TEXTURE::LOAD_FAILED\n";
  }

  stbi_image_free(data);
}

Texture::Texture(
  const char **paths,
  Texture::Type type,
  GLint wrap_mode_s,
  GLint wrap_mode_t,
  GLint wrap_mode_r,
  GLint min_filter,
  GLint mag_filter,
  bool gen_mipmaps
) : _id(0), path(std::string(paths[0])), _type(type), _gl_type(GL_TEXTURE_CUBE_MAP) {
  glGenTextures(1, &_id);

  int width[6], height[6], n_channels[6];
  unsigned char *data[6];
  stbi_set_flip_vertically_on_load(false);

  load_status = 1;
  for (unsigned i = 0; i < 6 && load_status; i++) {
    data[i] = stbi_load(paths[i], &width[i], &height[i], &n_channels[i], 4);
    load_status = data[i] != nullptr;
  }

  if (load_status) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(_gl_type, _id);

    for (unsigned i = 0; i < 6; i++) {
      glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
        0,
        (GLint) GL_RGBA,
        width[i],
        height[i],
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data[i]
      );
      stbi_image_free(data[i]);
    }

    set_texture_params(wrap_mode_s, wrap_mode_t, wrap_mode_r, min_filter, mag_filter, gen_mipmaps);
  } else {
    std::cerr << "ERROR::TEXTURE::LOAD_FAILED\n";
  }

  glBindTexture(_gl_type, 0);
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
  glBindTexture(_gl_type, _id);
}

const std::string &Texture::image_path() const {
  return path;
}

void Texture::set_texture_params(
  GLint wrap_mode_s,
  GLint wrap_mode_t,
  GLint wrap_mode_r,
  GLint min_filter,
  GLint mag_filter,
  bool gen_mipmaps
) const {
  if (gen_mipmaps) glGenerateMipmap(_gl_type);

  glTexParameteri(_gl_type, GL_TEXTURE_WRAP_S, wrap_mode_s);
  glTexParameteri(_gl_type, GL_TEXTURE_WRAP_T, wrap_mode_t);
  glTexParameteri(_gl_type, GL_TEXTURE_WRAP_R, wrap_mode_r);
  glTexParameteri(_gl_type, GL_TEXTURE_MIN_FILTER, min_filter);
  glTexParameteri(_gl_type, GL_TEXTURE_MAG_FILTER, mag_filter);
}
