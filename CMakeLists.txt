cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

set(CMAKE_CXX_STANDARD 17)

# Find GLFW
find_package(glfw3 3.3 REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add GLM
include_directories(src/include)

# Add GLAD
add_library(glad src/include/glad/glad.c src/include/glad/glad.h)
target_include_directories(glad PUBLIC src/include/glad)


# Add executable
add_executable(snake src/main.cpp)

# Link libraries
target_link_libraries(snake PRIVATE glad glfw ${OPENGL_LIBRARIES})

# Specify include directories for the executable
target_include_directories(snake PRIVATE src/include)
