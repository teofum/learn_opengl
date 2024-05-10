#ifndef LEARN_OPENGL_PROGRAM_H
#define LEARN_OPENGL_PROGRAM_H

#include <glad/glad.h>

#include <iostream>
#include <vector>

#include <shader.h>

class Program {
private:
  unsigned _id;
  int link_status = 0;

public:
  Program();

  Program(const Shader &vertex_shader, const Shader &fragment_shader);

  Program(const char *vertex_src, const char *fragment_src);

  ~Program();

  void attach_shader(const Shader &shader) const;

  void link();

  unsigned id() const;

  bool ready() const;

  void use() const;

  int uniform_location(const char *name) const;

  unsigned attrib_location(const char *name) const;
};

#endif //LEARN_OPENGL_PROGRAM_H