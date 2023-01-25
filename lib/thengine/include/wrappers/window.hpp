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

#include "vulkan_include.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>

namespace throttle {
namespace graphics {

class window_wrapper final {
private:
  struct glfw_window_deleter {
    void operator()(GLFWwindow *ptr) { glfwDestroyWindow(ptr); }
  };

private:
  std::unique_ptr<GLFWwindow, glfw_window_deleter> m_handle;

  vk::Extent2D m_extent;
  std::string  m_name;

public:
  window_wrapper(GLFWwindow *p_window, const std::string &p_name, const vk::Extent2D &p_extent)
      : m_handle{p_window}, m_extent{p_extent}, m_name{p_name} {}

  window_wrapper(const std::string &p_name, const vk::Extent2D &p_extent);

  GLFWwindow  *window() { return m_handle.get(); }
  vk::Extent2D extent() { return m_extent; }
};

} // namespace graphics
} // namespace throttle