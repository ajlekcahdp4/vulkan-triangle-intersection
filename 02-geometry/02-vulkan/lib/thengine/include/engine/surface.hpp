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

#include "instance.hpp"
#include "window.hpp"

namespace throttle::graphics {

class window_surface final {
public:
  window_surface(const vk::raii::Instance &, const std::string &, const vk::Extent2D &);

  vk::raii::SurfaceKHR &surface() { return m_handle; }
  GLFWwindow           *window() { return m_window_data->window(); }
  const vk::Extent2D   &extent() { return m_window_data->extent(); }

private:
  std::unique_ptr<i_window_data> m_window_data;
  vk::raii::SurfaceKHR           m_handle;
};

} // namespace throttle::graphics