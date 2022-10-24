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

namespace throttle {

namespace graphics {

class surface_data final {
public:
  surface_data(throttle::graphics::i_instance_data &p_instance_data, const std::string &p_name,
               const vk::Extent2D &p_extent)
      : m_window_data{p_name, p_extent}, m_handle{nullptr} {
    VkSurfaceKHR c_style_surface;
    auto         res = glfwCreateWindowSurface(*p_instance_data.instance(), window(), nullptr, &c_style_surface);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create a surface");
    m_handle = vk::raii::SurfaceKHR(p_instance_data.instance(), c_style_surface);
  }
  vk::raii::SurfaceKHR &surface() { return m_handle; }
  GLFWwindow           *window() { return m_window_data.window(); }

private:
  window_data          m_window_data;
  vk::raii::SurfaceKHR m_handle{nullptr};
};
} // namespace graphics
} // namespace throttle