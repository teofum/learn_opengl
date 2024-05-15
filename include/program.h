#ifndef LEARN_OPENGL_PROGRAM_H
#define LEARN_OPENGL_PROGRAM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include <shader.h>

using namespace glm;

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

  void set(const char *name, int value) const;

  void set(const char *name, float value) const;

  void set(const char *name, vec2 value) const;

  void set(const char *name, vec3 value) const;

  void set_matrix(const char *name, mat4 &mat) const;
};

#endif //LEARN_OPENGL_PROGRAM_H