/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <string>

#include <vulkan/vulkan_raii.hpp>

#include "wrappers/window.hpp"

namespace throttle::graphics {

using vk::Extent2D;

window_wrapper::window_wrapper(const std::string &p_name, const Extent2D &p_extent)
    : m_extent{p_extent}, m_name{p_name} {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  auto ptr = glfwCreateWindow(m_extent.width, m_extent.height, m_name.c_str(), nullptr, nullptr);
  if (!ptr) throw std::runtime_error("Failed to create a window");

  m_handle = {ptr, glfw_window_deleter{}};
}

}; // namespace throttle::graphics