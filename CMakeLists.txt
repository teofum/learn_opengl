cmake_minimum_required(VERSION 3.27)
project(learn_opengl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa -framework OpenGL -framework IOKit")

# Local includes
include_directories(include)

# GLFW
include_directories(/opt/homebrew/Cellar/glfw/3.4/include)
link_directories(/opt/homebrew/Cellar/glfw/3.4/lib)

# GLM
include_directories(/opt/homebrew/Cellar/glm/1.0.1/include)
link_directories(/opt/homebrew/Cellar/glm/1.0.1/lib)

add_executable(basics
        src/01-basics.cpp
        glad/glad.c
        stb/stb_image.cpp
        src/shader.cpp
        src/program.cpp
        src/texture.cpp
        src/object.cpp
        src/instance.cpp
        src/camera.cpp
        src/window.cpp
        src/light.cpp)
target_link_libraries(basics glfw)

add_executable(lighting
        src/02-lighting.cpp
        glad/glad.c
        stb/stb_image.cpp
        src/shader.cpp
        src/program.cpp
        src/texture.cpp
        src/object.cpp
        src/instance.cpp
        src/camera.cpp
        src/window.cpp
        src/light.cpp)
target_link_libraries(lighting glfw)
