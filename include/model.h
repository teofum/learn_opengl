#ifndef LEARN_OPENGL_MODEL_H
#define LEARN_OPENGL_MODEL_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include <mesh.h>
#include <texture.h>

class Model {
private:
  std::vector<Mesh> meshes;
  std::vector<Texture> loaded_textures;
  std::string directory;

  void process_node(const aiNode *node, const aiScene *scene);

  Mesh process_mesh(const aiMesh *mesh, const aiScene *scene);

  void load_material_textures(const aiMaterial *material, aiTextureType type, std::vector<Texture> &textures);

  void load_texture(const std::string &path, Texture::Type type, std::vector<Texture> &textures);

public:
  bool cull_backfaces = true;

  explicit Model(const std::string &file_path);

  void draw(const Program &program) const;
};

#endif //LEARN_OPENGL_MODEL_H
