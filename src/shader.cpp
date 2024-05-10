#include <shader.h>

Shader::Shader(GLenum type, const char *src_path)
  : _type(type) {
  std::string shader_src;
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    file.open(src_path);
    std::stringstream ss;

    ss << file.rdbuf();
    file.close();

    shader_src = ss.str();
  } catch (std::ifstream::failure &e) {
    std::cerr << "ERROR::SHADER::" << get_type() << "::FILE_READ_FAILED\n";
  }
  
  const char *shader_src_cstr = shader_src.c_str();

  _id = glCreateShader(type);
  glShaderSource(_id, 1, &shader_src_cstr, nullptr);
  glCompileShader(_id);

  glGetShaderiv(_id, GL_COMPILE_STATUS, &compile_status);
  if (!compile_status) {
    char info_log[512];
    glGetShaderInfoLog(_id, 512, nullptr, info_log);
    std::cerr << "ERROR::SHADER::" << get_type() << "::COMPILATION_FAILED\n"
              << info_log << "\n";
  }
}

Shader::~Shader() {
  glDeleteShader(_id);
}

std::string Shader::get_type() const {
  return _type == GL_FRAGMENT_SHADER ? "FRAGMENT" : "VERTEX";
}

Shader Shader::vertex(const char *src_path) {
  return {GL_VERTEX_SHADER, src_path};
}

Shader Shader::fragment(const char *src_path) {
  return {GL_FRAGMENT_SHADER, src_path};
}

unsigned Shader::id() const {
  return _id;
}

GLenum Shader::type() const {
  return _type;
}

bool Shader::ready() const {
  return compile_status != 0;
}
