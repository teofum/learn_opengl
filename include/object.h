#ifndef LEARN_OPENGL_OBJECT_H
#define LEARN_OPENGL_OBJECT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <program.h>

using namespace glm;

struct Vertex {
  vec3 position, normal;
  vec2 uv;
};

class Object {
private:
  unsigned vao, vbo, ebo;
  std::vector<Vertex> vertex_data;
  std::vector<unsigned> vertex_indices;

  void load_obj(const char *obj_path);

  void build_vertex_data(
    const std::vector<vec3> &vertices,
    const std::vector<vec3> &normals,
    const std::vector<vec2> &uvs,
    const std::vector<std::vector<u32vec3>> &faces
  );

public:
  Object(
    const char *obj_path,
    const Program &program,
    const char *pos_name,
    const char *normal_name,
    const char *uv_name
  );

  Object(const char *obj_path, const Program &program) : Object(obj_path, program, "aPos", "aNormal", "aTexCoord") {
  };

  void draw() const;
};

#endif // LEARN_OPENGL_OBJECT_H
