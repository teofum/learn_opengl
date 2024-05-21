#include <model.h>

Model::Model(const std::string &file_path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
    file_path,
    aiProcess_Triangulate | aiProcess_FlipUVs
  );

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << "\n";
    return;
  }

  directory = file_path.substr(0, file_path.find_last_of('/'));
  process_node(scene->mRootNode, scene);
}

void Model::process_node(const aiNode *node, const aiScene *scene) { // NOLINT(*-no-recursion)
  for (unsigned i = 0; i < node->mNumMeshes; i++) {
    const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(process_mesh(mesh, scene));
  }

  for (unsigned i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene);
  }
}

Mesh Model::process_mesh(const aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned> indices;
  std::vector<Texture> textures;

  // Vertices
  for (unsigned i = 0; i < mesh->mNumVertices; i++) {
    const auto &pos = mesh->mVertices[i];
    const auto &normal = mesh->mNormals[i];
    Vertex vertex{
      vec3(pos.x, pos.y, pos.z),
      vec3(normal.x, normal.y, normal.z),
      vec2(0.0f, 0.0f)
    };

    if (mesh->mTextureCoords[0]) {
      vertex.uv.x = mesh->mTextureCoords[0][i].x;
      vertex.uv.y = mesh->mTextureCoords[0][i].y;
    }

    vertices.push_back(vertex);
  }

  // Indices
  for (unsigned i = 0; i < mesh->mNumFaces; i++) {
    const auto &face = mesh->mFaces[i];
    for (unsigned j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // Material
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    load_material_textures(material, aiTextureType_DIFFUSE, textures);
    load_material_textures(material, aiTextureType_SPECULAR, textures);
  }

  // We won't be using these vectors anymore, so we can move them instead of copying
  return {std::move(vertices), std::move(indices), std::move(textures)};
}

void Model::load_material_textures(
  const aiMaterial *material,
  aiTextureType type,
  std::vector<Texture> &textures
) {
  aiString str;
  for (unsigned i = 0; i < material->GetTextureCount(type); i++) {
    material->GetTexture(type, i, &str);
    std::stringstream ss;
    ss << directory << "/" << str.C_Str();
    std::string path = ss.str();

    Texture::Type tex_type = Texture::Type::Diffuse;
    if (type == aiTextureType_SPECULAR) tex_type = Texture::Type::Specular;

    load_texture(path, tex_type, textures);
  }
}

void Model::load_texture(const std::string &path, Texture::Type type, std::vector<Texture> &textures) {
  bool found = false;
  for (const Texture &loaded: loaded_textures) {
    if (loaded.image_path() == path) {
      textures.push_back(loaded);
      found = true;
      break;
    }
  }

  if (!found) {
    Texture texture(path.c_str(), type);
    textures.push_back(texture);
    loaded_textures.push_back(texture);
  }
}

void Model::draw(const Program &program) const {
  program.use();

  if (cull_backfaces)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);

  for (const auto &mesh: meshes) mesh.draw(program);
}

void Model::set_instance_attribute(unsigned int location, const std::vector<mat4> &data) const {
  unsigned buf;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferData(GL_ARRAY_BUFFER, (long) (data.size() * sizeof(mat4)), data.data(), GL_STATIC_DRAW);

  for (const auto &mesh: meshes) {
    mesh.set_instance_attribute(location);
  }
}

void Model::draw_instanced(const Program &program, unsigned int count) const {
  program.use();

  if (cull_backfaces)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);

  for (const auto &mesh: meshes) mesh.draw_instanced(program, count);
}
