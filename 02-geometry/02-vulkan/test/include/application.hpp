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

#include "device.hpp"
#include "instance.hpp"
#include "queue_families.hpp"
#include "surface.hpp"

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace triangles {
class application final {
public:
  application()
      : m_instance_data{std::make_unique<throttle::graphics::instance_data>()},
        m_phys_device{throttle::graphics::pick_physical_device(m_instance_data->instance())},
        m_surface_data{std::make_unique<throttle::graphics::surface_data>(*m_instance_data, "Triangles intersection",
                                                                          vk::Extent2D{800, 600})},
        m_logical_device{throttle::graphics::create_device(m_phys_device, *m_surface_data)} {}

  void run() {
    while (!glfwWindowShouldClose(m_surface_data->window()))
      glfwPollEvents();
  }

private:
  std::unique_ptr<throttle::graphics::i_instance_data> m_instance_data{nullptr};
  vk::raii::PhysicalDevice         m_phys_device{nullptr};
  std::unique_ptr<throttle::graphics::i_surface_data>  m_surface_data{nullptr};
  vk::raii::Device                                     m_logical_device{nullptr};
};
} // namespace triangles