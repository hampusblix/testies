cmake_minimum_required(VERSION 3.18)
project(MiniFpsEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

add_executable(mini_fps_engine
  src/main.cpp
  src/Window.cpp
  src/Shader.cpp
  src/Terrain.cpp
  src/PlayerController.cpp
  src/Renderer.cpp
)

target_include_directories(mini_fps_engine PRIVATE src)
target_link_libraries(mini_fps_engine PRIVATE OpenGL::GL glfw glm::glm GLEW::GLEW)
