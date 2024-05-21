#ifndef LEARN_OPENGL_MESH_H
#define LEARN_OPENGL_MESH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <program.h>
#include <texture.h>

#define DEFAULT_POS_LOCATION 0
#define DEFAULT_NORMAL_LOCATION 1
#define DEFAULT_UV_LOCATION 2

using namespace glm;

struct Vertex {
  vec3 position, normal;
  vec2 uv;
};

class Mesh {
private:
  unsigned vao, vbo, ebo;
  size_t vertex_count;
  std::vector<Vertex> vertex_data;
  std::vector<unsigned> vertex_indices;
  std::vector<Texture> textures;

  void init(unsigned pos_location, unsigned normal_location, unsigned uv_location);

  void bind_textures(const Program &program) const;

public:
  Mesh(
    const std::vector<Vertex> &vertices,
    const std::vector<unsigned int> &indices,
    const std::vector<Texture> &textures,
    unsigned pos_location = DEFAULT_POS_LOCATION,
    unsigned normal_location = DEFAULT_NORMAL_LOCATION,
    unsigned uv_location = DEFAULT_UV_LOCATION
  );

  Mesh(
    std::vector<Vertex> &&vertices,
    std::vector<unsigned int> &&indices,
    std::vector<Texture> &&textures,
    unsigned pos_location = DEFAULT_POS_LOCATION,
    unsigned normal_location = DEFAULT_NORMAL_LOCATION,
    unsigned uv_location = DEFAULT_UV_LOCATION
  );

  void draw(const Program &program) const;

  void draw_instanced(const Program &program, unsigned count) const;

  void set_instance_attribute(unsigned location) const;
};

#endif // LEARN_OPENGL_MESH_H
