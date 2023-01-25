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

#include "instance.hpp"
#include "window.hpp"

namespace throttle::graphics {

class window_surface final {
private:
  std::unique_ptr<window_wrapper> m_window = nullptr;
  vk::raii::SurfaceKHR            m_handle = nullptr;

public:
  window_surface(const vk::raii::Instance &, const std::string &, const vk::Extent2D &);

  vk::raii::SurfaceKHR &surface() { return m_handle; }
  GLFWwindow           *window() { return m_window->window(); }

  vk::Extent2D extent() { return m_window->extent(); }
};

} // namespace throttle::graphics