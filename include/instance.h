#ifndef LEARN_OPENGL_INSTANCE_H
#define LEARN_OPENGL_INSTANCE_H

#include <glm/glm.hpp>

#include <object.h>
#include <program.h>

using namespace glm;

class Instance {
private:
  const Object &obj;
  const Program &program;

public:
  mat4 transform = mat4(1.0);

  Instance(const Object &obj, const Program &program);

  void draw(const char *model_matrix_name = "model") const;
};

#endif //LEARN_OPENGL_INSTANCE_H
