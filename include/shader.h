#ifndef LEARN_OPENGL_SHADER_H
#define LEARN_OPENGL_SHADER_H

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class Shader {
private:
  unsigned _id;
  int compile_status = 0;
  GLenum _type;

  std::string get_type() const;

public:
  Shader(GLenum type, const char *src_path);

  ~Shader();

  static Shader vertex(const char *src_path);

  static Shader geometry(const char *src_path);

  static Shader fragment(const char *src_path);

  unsigned id() const;

  GLenum type() const;

  bool ready() const;
};

#endif //LEARN_OPENGL_SHADER_H
