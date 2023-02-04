cmake_minimum_required(VERSION 3.24)

project(headless-rendering)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)

add_subdirectory(glad)
add_subdirectory(stb_image)

add_executable(egl-gles-nvidia egl-gles-nvidia-main.cpp render.cpp render.hpp shader.hpp)
target_link_libraries(egl-gles-nvidia PRIVATE glad stbimage OpenGL::OpenGL OpenGL::EGL "dl")

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/egl-gles-nvidia DESTINATION .)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION .)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/vertex.vs ${CMAKE_CURRENT_SOURCE_DIR}/fragment.fs DESTINATION .)