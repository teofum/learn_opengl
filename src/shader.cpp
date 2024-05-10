#include <shader.h>

unsigned compile_shader(GLenum type, const char *src, int *success) {
  unsigned shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, success);
  if (!(*success)) {
    char info_log[512];
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    std::cerr << "ERROR::SHADER::"
              << (type == GL_FRAGMENT_SHADER ? "FRAGMENT" : "VERTEX")
              << "::COMPILATION_FAILED\n" << info_log << "\n";
  }

  return shader;
}
