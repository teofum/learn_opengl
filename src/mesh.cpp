#include <mesh.h>
#include <map>

using namespace glm;

Mesh::Mesh(
  const std::vector<Vertex> &vertices,
  const std::vector<unsigned int> &indices,
  const std::vector<Texture> &textures,
  unsigned int pos_location,
  unsigned int normal_location,
  unsigned int uv_location
) : vertex_data(vertices), vertex_indices(indices), textures(textures) {
  vertex_count = vertex_indices.size();
  vao = vbo = ebo = 0;

  init(pos_location, normal_location, uv_location);
}

Mesh::Mesh(
  std::vector<Vertex> &&vertices,
  std::vector<unsigned int> &&indices,
  std::vector<Texture> &&textures,
  unsigned int pos_location,
  unsigned int normal_location,
  unsigned int uv_location
) : vertex_data(vertices), vertex_indices(indices), textures(textures) {
  vertex_count = vertex_indices.size();
  vao = vbo = ebo = 0;

  init(pos_location, normal_location, uv_location);
}

void Mesh::init(unsigned int pos_location, unsigned int normal_location, unsigned int uv_location) {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  // Vertex Array Object
  glBindVertexArray(vao);

  // Vertex Buffer Object
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    vertex_data.size() * sizeof(Vertex), // NOLINT(*-narrowing-conversions)
    vertex_data.data(),
    GL_STATIC_DRAW
  );

  // Element Buffer Object
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    vertex_indices.size() * sizeof(unsigned), // NOLINT(*-narrowing-conversions)
    vertex_indices.data(),
    GL_STATIC_DRAW
  );

  // Vertex positions
  glEnableVertexAttribArray(pos_location);
  glVertexAttribPointer(pos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));

  // Vertex normals
  glEnableVertexAttribArray(normal_location);
  glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));

  // Vertex UVs
  glEnableVertexAttribArray(uv_location);
  glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, uv));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Mesh::bind_textures(const Program &program) const {
  unsigned diffuse = 0, specular = 0;
  for (const auto &texture: textures) {
    unsigned char i = diffuse + specular;

    texture.bind(i);
    std::string uniform_name;
    switch (texture.type()) {
      case Texture::Type::Diffuse:
        uniform_name = "material.diffuse" + std::to_string(diffuse++);
        break;
      case Texture::Type::Specular:
        uniform_name = "material.specular" + std::to_string(specular++);
        break;
    }

    program.set(uniform_name.c_str(), (int) i);
  }
}

void Mesh::draw(const Program &program) const {
  bind_textures(program);

  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, 0); // NOLINT(*)
  glBindVertexArray(0);
}

void Mesh::draw_instanced(const Program &program, unsigned int count) const {
  bind_textures(program);

  glBindVertexArray(vao);
  glDrawElementsInstanced(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, 0, count); // NOLINT(*)
  glBindVertexArray(0);
}

void Mesh::set_instance_attribute(unsigned int location) const {
  glBindVertexArray(vao);
  size_t vec4_size = sizeof(vec4);

  for (int i = 0; i < 4; i++) {
    glEnableVertexAttribArray(location + i);
    glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, (int) (4 * vec4_size), (void *) (i * vec4_size));
    glVertexAttribDivisor(location + i, 1);
  }

  glBindVertexArray(0);
}
