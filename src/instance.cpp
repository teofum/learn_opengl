#include <glm/gtc/type_ptr.hpp>

#include <instance.h>

Instance::Instance(const Object &obj, const Program &program)
  : obj(obj), program(program) {
}

void Instance::draw(const char *model_matrix_name) const {
  program.use();
  int loc_model = program.uniform_location(model_matrix_name);
  glUniformMatrix4fv(loc_model, 1, GL_FALSE, value_ptr(transform));
  obj.draw();
}
