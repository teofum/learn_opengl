#include <program.h>

Program::Program() {
  _id = glCreateProgram();
}

Program::Program(const Shader &vertex_shader, const Shader &fragment_shader)
  : Program() {
  attach_shader(vertex_shader);
  attach_shader(fragment_shader);
  link();
}

Program::Program(const char *vertex_src, const char *fragment_src)
  : Program() {
  attach_shader(Shader::vertex(vertex_src));
  attach_shader(Shader::fragment(fragment_src));
  link();
}

Program::~Program() {
  glDeleteProgram(_id);
}

void Program::attach_shader(const Shader &shader) const {
  glAttachShader(_id, shader.id());
}

void Program::link() {
  glLinkProgram(_id);
  glGetProgramiv(_id, GL_LINK_STATUS, &link_status);
  if (!link_status) {
    char info_log[512];
    glGetProgramInfoLog(_id, 512, nullptr, info_log);
    std::cerr << "ERROR::PROGRAM::COMPILATION_FAILED\n" << info_log << "\n";
  }
}

unsigned Program::id() const {
  return _id;
}

bool Program::ready() const {
  return link_status != 0;
}

void Program::use() const {
  glUseProgram(_id);
}

int Program::uniform_location(const char *name) const {
  return glGetUniformLocation(_id, name);
}

unsigned Program::attrib_location(const char *name) const {
  return glGetAttribLocation(_id, name);
}

