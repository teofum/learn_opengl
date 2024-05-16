#ifndef LEARN_OPENGL_TEXTURE_H
#define LEARN_OPENGL_TEXTURE_H

class Texture {
public:
  enum Type {
    Diffuse = 0, Specular
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
  );

  Texture(const char *path, Type type, GLint wrap_mode);

  Texture(const char *path, Type type);

  Texture(const Texture &tex) = default;

  ~Texture();

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
};

#endif //LEARN_OPENGL_TEXTURE_H
