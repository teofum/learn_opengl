#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>

#define WIDTH 800
#define HEIGHT 600

constexpr const char *vertex_src = "#version 330 core\n"
                                   "in vec3 aPos;\n"
                                   "in vec3 aColor;\n"
                                   "\n"
                                   "out vec3 color;\n"
                                   "\n"
                                   "void main() {\n"
                                   "    color = aColor;\n"
                                   "    gl_Position = vec4(aPos, 1.0);\n"
                                   "}";

constexpr const char *fragment_src = "#version 330 core\n"
                                     "out vec4 FragColor;\n"
                                     "in vec3 color;\n"
                                     "\n"
                                     "void main() {\n"
                                     "    FragColor = vec4(color, 1.0f);\n"
                                     "}";

void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
  glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create GLFW window
  GLFWwindow *window = glfwCreateWindow(
    WIDTH, HEIGHT, "Learn OpenGL",
    nullptr, nullptr
  );
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwTerminate();
    return -1;
  }

  // Init viewport
  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // Compile shaders and link program
  // --------------------------------------------
  int success;
  unsigned vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_src, &success);
  if (!success) {
    glfwTerminate();
    return -1;
  }

  unsigned fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_src, &success);
  if (!success) {
    glfwTerminate();
    return -1;
  }

  unsigned program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetProgramInfoLog(program, 512, nullptr, info_log);
    std::cerr << "ERROR::PROGRAM::COMPILATION_FAILED\n" << info_log << "\n";

    glfwTerminate();
    return -1;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Setup vertex data
  // --------------------------------------------
  float vertices[] = {
    0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
  };
  unsigned indices[] = {
    0, 1, 3,
    1, 2, 3,
  };

  // Setup buffers
  // --------------------------------------------
  unsigned vao, vbo, ebo;

  // Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Vertex Buffer Object
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Element Buffer Object
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  unsigned loc = glGetAttribLocation(program, "aPos");
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0); // NOLINT(*-use-nullptr)
  glEnableVertexAttribArray(loc);

  loc = glGetAttribLocation(program, "aColor");
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(loc);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

//  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  int color_location = glGetUniformLocation(program, "color");

  // Rendering loop
  // --------------------------------------------
  while (!glfwWindowShouldClose(window)) {
    process_input(window);

    // Rendering code
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto time = (float) glfwGetTime();
    float green = 0.5f + 0.5f * sinf(time);

    glUseProgram(program);
    glUniform3f(color_location, 0.0f, green, 0.0f);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
