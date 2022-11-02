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

#include "engine/surface.hpp"

namespace throttle::graphics {

using vk::Extent2D;
using vk::raii::Instance;

window_surface::window_surface(const Instance &p_instance, const std::string &p_name, const Extent2D &p_extent)
    : m_window_data{std::make_unique<window_data>(p_name, p_extent)}, m_handle{nullptr} {
  VkSurfaceKHR c_style_surface;
  auto         res = glfwCreateWindowSurface(*p_instance, window(), nullptr, &c_style_surface);
  if (res != VK_SUCCESS) throw std::runtime_error("Failed to create a surface");
  m_handle = vk::raii::SurfaceKHR(p_instance, c_style_surface);
}

}; // namespace throttle::graphics