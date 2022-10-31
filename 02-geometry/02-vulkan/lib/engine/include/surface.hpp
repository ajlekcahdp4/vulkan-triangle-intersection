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

struct i_surface_data {
  virtual vk::raii::SurfaceKHR &surface() = 0;
  virtual GLFWwindow           *window() = 0;
  virtual const vk::Extent2D   &extent() = 0;
  virtual ~i_surface_data() {}
};

class surface_data final : public i_surface_data {
public:
  surface_data(const vk::raii::Instance &p_instance, const std::string &p_name, const vk::Extent2D &p_extent)
      : m_window_data{std::make_unique<window_data>(p_name, p_extent)}, m_handle{nullptr} {
    VkSurfaceKHR c_style_surface;
    auto         res = glfwCreateWindowSurface(*p_instance, window(), nullptr, &c_style_surface);
    if (res != VK_SUCCESS) throw std::runtime_error("Failed to create a surface");
    m_handle = vk::raii::SurfaceKHR(p_instance, c_style_surface);
  }

  vk::raii::SurfaceKHR &surface() override { return m_handle; }
  GLFWwindow           *window() override { return m_window_data->window(); }
  const vk::Extent2D   &extent() override { return m_window_data->extent(); }

private:
  std::unique_ptr<i_window_data> m_window_data{nullptr};
  vk::raii::SurfaceKHR m_handle{nullptr};
};
} // namespace graphics
} // namespace throttle