#ifndef LEARN_OPENGL_SHADER_H
#define LEARN_OPENGL_SHADER_H

#include <iostream>
#include <glad/glad.h>

unsigned compile_shader(GLenum type, const char *src, int *success);

#endif //LEARN_OPENGL_SHADER_H
