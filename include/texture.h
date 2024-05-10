#ifndef LEARN_OPENGL_TEXTURE_H
#define LEARN_OPENGL_TEXTURE_H

class Texture {
private:
  int load_status;
  unsigned _id;

public:
  Texture(
    const char *img_path,
    GLenum format,
    GLint wrap_mode_s,
    GLint wrap_mode_t,
    GLint min_filter,
    GLint mag_filter,
    bool gen_mipmaps
  );

  Texture(const char *img_path, GLenum format, GLint wrap_mode, GLint min_filter, GLint mag_filter, bool gen_mipmaps);

  Texture(const char *img_path, GLenum format, GLint wrap_mode);

  explicit Texture(const char *img_path, GLenum format = GL_RGB);

  ~Texture();

  unsigned id() const;

  bool ready() const;

  void bind(unsigned char texture_unit = 0) const;
};

#endif //LEARN_OPENGL_TEXTURE_H
