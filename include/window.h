#ifndef LEARN_OPENGL_WINDOW_H
#define LEARN_OPENGL_WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

GLFWwindow *init_window(int initial_width, int initial_height, const char *title);

#endif //LEARN_OPENGL_WINDOW_H
