cmake_minimum_required(VERSION 3.27)
project(learn_opengl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa -framework OpenGL -framework IOKit")

include_directories(include)
include_directories(/opt/homebrew/Cellar/glfw/3.4/include)
link_directories(/opt/homebrew/Cellar/glfw/3.4/lib)

add_executable(learn_opengl
        src/main.cpp
        glad/glad.c
        src/shader.cpp
        src/program.cpp)
target_link_libraries(learn_opengl glfw)
