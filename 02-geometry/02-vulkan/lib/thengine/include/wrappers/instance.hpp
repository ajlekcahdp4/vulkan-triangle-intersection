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
#include "logging.hpp"
#include <GLFW/glfw3.h>

namespace throttle::graphics {

std::vector<const char *> get_required_extensions();

inline std::vector<const char *> get_required_layers() {
  return std::vector<const char *>{"VK_LAYER_KHRONOS_validation"};
}

bool extensions_supported(const std::vector<const char *> &e, const vk::raii::Context &);
bool layers_supported(const std::vector<const char *> &layers);
bool is_supported(const std::vector<const char *> &e, const std::vector<const char *> &l, const vk::raii::Context &);

// concrete instance class with debug messenger.
class instance_wrapper {
private:
  vk::raii::Instance               m_handle = nullptr;
  vk::raii::DebugUtilsMessengerEXT m_dmessenger = nullptr;

public:
  instance_wrapper() : m_handle{create_instance()}, m_dmessenger{create_debug_messenger(m_handle)} {}

  vk::raii::Instance &instance() { return m_handle; }

private:
  vk::raii::Instance create_instance();
};

} // namespace throttle::graphics