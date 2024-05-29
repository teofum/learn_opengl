#ifndef LEARN_OPENGL_TEXTURE_H
#define LEARN_OPENGL_TEXTURE_H

class Texture {
public:
  enum Type {
    Diffuse = 0, Specular, Normal
  };

  Texture(
    const char *path,
    Type type,
    GLint wrap_mode_s,
    GLint wrap_mode_t,
    GLint min_filter,
    GLint mag_filter,
    bool gen_mipmaps
  );

  Texture(
    const char *path,
    Type type,
    GLint wrap_mode,
    GLint min_filter,
    GLint mag_filter,
    bool gen_mipmaps
  )
    : Texture(path, type, wrap_mode, wrap_mode, min_filter, mag_filter, gen_mipmaps) {
  }

  Texture(const char *path, Type type, GLint wrap_mode)
    : Texture(path, type, wrap_mode, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true) {
  };

  Texture(const char *path, Type type) : Texture(path, type, GL_REPEAT) {
  };

  Texture(
    const char *paths[6],
    Type type,
    GLint wrap_mode_s,
    GLint wrap_mode_t,
    GLint wrap_mode_r,
    GLint min_filter,
    GLint mag_filter,
    bool gen_mipmaps
  );

  Texture(
    const char *paths[6],
    Type type,
    GLint wrap_mode,
    GLint min_filter,
    GLint mag_filter,
    bool gen_mipmaps
  )
    : Texture(paths, type, wrap_mode, wrap_mode, wrap_mode, min_filter, mag_filter, gen_mipmaps) {
  }

  Texture(const char *paths[6], Type type, GLint wrap_mode)
    : Texture(paths, type, wrap_mode, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true) {
  };

  Texture(const char *paths[6], Type type) : Texture(paths, type, GL_CLAMP_TO_EDGE) {
  };

  Texture(const Texture &tex) = default;

  unsigned id() const;

  Type type() const;

  const std::string &image_path() const;

  bool ready() const;

  void bind(unsigned char texture_unit = 0) const;

private:
  int load_status;
  unsigned _id;
  std::string path;
  Type _type;
  GLenum _gl_type;

  void set_texture_params(
    GLint wrap_mode_s,
    GLint wrap_mode_t,
    GLint wrap_mode_r,
    GLint min_filter,
    GLint mag_filter,
    bool gen_mipmaps
  ) const;
};

#endif //LEARN_OPENGL_TEXTURE_H
