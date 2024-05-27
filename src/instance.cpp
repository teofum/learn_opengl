#include <glm/gtc/type_ptr.hpp>

#include <instance.h>

Instance::Instance(const Model &obj, const Program &program)
  : obj(obj), program(program) {
}

void Instance::draw(const char *model_matrix_name) const {
  program.use();
  int loc_model = program.uniform_location(model_matrix_name);
  glUniformMatrix4fv(loc_model, 1, GL_FALSE, value_ptr(transform));
  obj.draw(program);
}

void Instance::draw_with(const Program &prog, const char *model_matrix_name) const {
  prog.use();
  int loc_model = prog.uniform_location(model_matrix_name);
  glUniformMatrix4fv(loc_model, 1, GL_FALSE, value_ptr(transform));
  obj.draw(prog);
}
