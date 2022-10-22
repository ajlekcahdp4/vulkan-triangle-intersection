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

#include "instance.hpp"
#include "logging.hpp"
#include "window.hpp"

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace triangles {
class application final {
public:
  application()
      : m_window{throttle::graphics::create_window(800, 600)}, m_instance{throttle::graphics::create_instance(
                                                                   "best instance")},
        m_debug_messenger{throttle::graphics::create_debug_messenger(m_instance)} {}

  void run() {}

private:
  GLFWwindow                      *m_window{nullptr};
  vk::raii::Instance               m_instance{nullptr};
  vk::raii::DebugUtilsMessengerEXT m_debug_messenger{nullptr};
};
} // namespace triangles