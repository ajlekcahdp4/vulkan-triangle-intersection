/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <exception>

namespace throttle {
namespace graphics {
inline GLFWwindow *create_window(const uint32_t p_width, const uint32_t p_height) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  GLFWwindow *window = glfwCreateWindow(p_width, p_height, "Triangles", nullptr, nullptr);
  if (!window) throw std::runtime_error("Failed to create a window");
  return window;
}
} // namespace graphics
} // namespace throttle