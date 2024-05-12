#include <object.h>
#include <map>

using namespace glm;

namespace {
// Internal use struct
struct VertexIndices {
  unsigned i_pos, i_norm, i_uv;
};

bool operator==(const VertexIndices &lhs, const VertexIndices &rhs) {
  return lhs.i_pos == rhs.i_pos && lhs.i_norm == rhs.i_norm && lhs.i_uv == rhs.i_uv;
}

bool operator<(const VertexIndices &lhs, const VertexIndices &rhs) {
  return lhs.i_pos < rhs.i_pos || (
    lhs.i_pos == rhs.i_pos && (
      lhs.i_norm < rhs.i_norm || (
        lhs.i_norm == rhs.i_norm && lhs.i_uv < rhs.i_uv
      )
    )
  );
}
}

Object::Object(
  const char *obj_path,
  const Program &program,
  const char *pos_name,
  const char *normal_name,
  const char *uv_name
) {
  vao = vbo = ebo = 0;

  load_obj(obj_path);

  // Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Vertex Buffer Object
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    vertex_data.size() * sizeof(Vertex), // NOLINT(*-narrowing-conversions)
    vertex_data.data(),
    GL_STATIC_DRAW
  );

  // Element Buffer Object
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    vertex_indices.size() * sizeof(unsigned), // NOLINT(*-narrowing-conversions)
    vertex_indices.data(),
    GL_STATIC_DRAW
  );

  unsigned loc = program.attrib_location(pos_name);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0); // NOLINT(*-use-nullptr)
  glEnableVertexAttribArray(loc);

  loc = program.attrib_location(normal_name);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(loc);

  loc = program.attrib_location(uv_name);
  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
  glEnableVertexAttribArray(loc);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Object::load_obj(const char *obj_path) {
  std::string shader_src;
  std::ifstream file(obj_path);

  std::vector<vec3> vertices, normals;
  std::vector<vec2> uvs;
  std::vector<std::vector<u32vec3>> faces;

  std::string line, type;
  bool is_normal;
  unsigned line_number = 1;

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    ss >> type;

    if ((is_normal = type == "vn") || type == "v") {
      float x, y, z;
      ss >> x >> y >> z;

      if (is_normal) normals.emplace_back(x, y, z);
      else vertices.emplace_back(x, y, z);
    } else if (type == "vt") {
      float u, v;
      ss >> u >> v;
      uvs.emplace_back(u, v);
    } else if (type == "f") {
      std::string vertex_str;
      std::vector<u32vec3> face;

      while (ss >> vertex_str && face.size() < 3) {
        std::stringstream vss(vertex_str);
        std::string part;
        unsigned indices[3];
        unsigned i = 0;

        while (getline(vss, part, '/')) {
          // Subtract one because OBJ indices are one-indexed
          indices[i++] = (unsigned) stoi(part) - 1;
        }
        face.emplace_back(indices[0], indices[1], indices[2]);
      }

      if (face.size() != 3) {
        std::cerr << "WARN::OBJECT::DEGENERATE_FACE\n" << face.size() << " at line " << line_number << "\n";
      }

      faces.push_back(face);
    } else {
      std::cerr << "WARN::OBJECT::UNRECOGNIZED_TYPE\n" << type << " at line " << line_number << "\n";
    }

    line_number++;
  }

  build_vertex_data(vertices, normals, uvs, faces);

  file.close();
}

void Object::build_vertex_data(
  const std::vector<vec3> &vertices,
  const std::vector<vec3> &normals,
  const std::vector<vec2> &uvs,
  const std::vector<std::vector<u32vec3>> &faces
) {
  // We need to combine three different buffers for vertex positions/normals/uvs
  // into a single buffer containing all vertex attributes, because that's how
  // OpenGL works

  // This is non-trivial if we want to avoid repeating vertices, as we need to
  // keep track of which identical position/normal/uv combinations we already
  // have in our vertex attribs buffer

  // Cache of already indexed pos/normal/uv combinations
  std::map<VertexIndices, unsigned> indexed;

  for (const auto &face: faces) {
    for (const auto &v_indices: face) {
      VertexIndices indices{v_indices.x, v_indices.y, v_indices.z};
      if (indexed.contains(indices)) {
        // This vertex is already present in data buffer, simply add the index
        unsigned idx = indexed.at(indices);
        vertex_indices.push_back(idx);
      } else {
        // Vertex with this combination of indices is not in data buffer, add it
        Vertex vertex{
          vertices[v_indices.x],
          normals[v_indices.y],
          uvs[v_indices.z],
        };
        unsigned idx = vertex_data.size();
        vertex_data.push_back(vertex);
        vertex_indices.push_back(idx);

        // And cache the index combination in case it appears again
        indexed.insert({indices, idx});
      }
    }
  }
}

void Object::draw() const {
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, vertex_indices.size(), GL_UNSIGNED_INT, 0); // NOLINT(*)
  glBindVertexArray(0);
}
