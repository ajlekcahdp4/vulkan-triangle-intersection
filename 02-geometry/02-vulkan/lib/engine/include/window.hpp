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

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <exception>

namespace throttle {
namespace graphics {
struct i_window_data {
  virtual GLFWwindow *window() = 0;
  virtual ~i_window_data() {}
};

class window_data final : public i_window_data {
public:
  window_data(GLFWwindow *p_window, const std::string &p_name, const vk::Extent2D &p_extent)
      : m_handle{p_window}, m_extent{p_extent}, m_name{p_name} {}
  window_data(const window_data &) = delete;
  window_data(window_data &&rhs) {
    std::swap(m_handle, rhs.m_handle);
    std::swap(m_extent, rhs.m_extent);
    std::swap(m_name, rhs.m_name);
  }

  window_data(const std::string p_name, const vk::Extent2D &p_extent) : m_extent{p_extent}, m_name{p_name} {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_handle = glfwCreateWindow(m_extent.width, m_extent.height, m_name.c_str(), nullptr, nullptr);
    if (!m_handle) throw std::runtime_error("Failed to create a window");
  }

  GLFWwindow *window() { return m_handle; }
  ~window_data() { glfwDestroyWindow(m_handle); }

private:
  GLFWwindow  *m_handle{nullptr};
  vk::Extent2D m_extent;
  std::string  m_name;
};

} // namespace graphics
} // namespace throttle