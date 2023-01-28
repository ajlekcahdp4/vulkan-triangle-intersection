/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "ezvk/instance.hpp"
#include "ezvk/window.hpp"

namespace triangles {

inline auto required_vk_extensions() {
  auto glfw_extensions = ezvk::glfw_required_vk_extensions();
  glfw_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  return glfw_extensions;
}

inline std::vector<std::string> required_vk_layers(bool validation = false) {
  if (validation) return {"VK_LAYER_KHRONOS_validation"};
  return {};
}

}; // namespace triangles