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
#include "logging.hpp"
#include <GLFW/glfw3.h>

namespace throttle {
namespace graphics {

// returns vector of the required extensions.
inline std::vector<const char *> get_required_extensions() {
  uint32_t                  ext_count{};
  auto                      arr_glfw_extensions = glfwGetRequiredInstanceExtensions(&ext_count);
  std::vector<const char *> extensions{arr_glfw_extensions, arr_glfw_extensions + ext_count};
  extensions.push_back("VK_EXT_debug_utils");
  return extensions;
}

// returns vector of the requested validation layers.
inline std::vector<const char *> get_required_layers() {
#if defined(ENABLE_VALIDATION_LAYERS)
  return std::vector<const char *>{"VK_LAYER_KHRONOS_validation"};
#else
  return {};
#endif
}

// check if we can support all the required extensions in current context.
inline bool extensions_supported(const std::vector<const char *> &extensions, const vk::raii::Context &context) {
  auto supported_extensions = context.enumerateInstanceExtensionProperties();
  bool found{};
  for (auto extension : extensions) {
    found = false;
    for (auto &s_extension : supported_extensions)
      if (std::strcmp(extension, s_extension.extensionName) == 0) found = true;
    if (!found) return false;
  }
  return true;
}

// check if we can support all the layers.
inline bool layers_supported(const std::vector<const char *> &layers) {
  bool found{};
  auto supported_layers = vk::enumerateInstanceLayerProperties();
  for (auto layer : layers) {
    found = false;
    for (auto &s_layer : supported_layers)
      if (std::strcmp(layer, s_layer.layerName) == 0) found = true;
    if (!found) return false;
  }
  return true;
}

// check if we can support all the layer and extensions in current context.
inline bool is_supported(const std::vector<const char *> &extensions, const std::vector<const char *> &layers,
                         const vk::raii::Context &context) {
  return extensions_supported(extensions, context) && layers_supported(layers);
}

// instance class with debug messenger.
class instance_data final {
public:
  instance_data() : m_handle{m_context, create_info(m_context)}, m_dmessenger{create_debug_messenger(m_handle)} {}
  vk::raii::Instance &instance() { return m_handle; }

private:
  static vk::InstanceCreateInfo create_info(vk::raii::Context &p_context) {
    glfwInit();
    auto                version = p_context.enumerateInstanceVersion();
    vk::ApplicationInfo app_info{"Triangles intersection", version, "Best engine", version, version};
    auto                extensions = get_required_extensions();
    auto                layers = get_required_layers();
    if (!is_supported(extensions, layers, p_context))
      throw std::runtime_error("Not all the requested extensions are supported");
    // clang-format off
    vk::InstanceCreateInfo create_info {
      vk::InstanceCreateFlags{},
      &app_info, static_cast<uint32_t>(layers.size()),
      layers.data(),
      static_cast<uint32_t>(extensions.size()),
      extensions.data()
    };
    // clang-format on
    return create_info;
  }

  vk::raii::Context                m_context;
  vk::raii::Instance               m_handle{nullptr};
  vk::raii::DebugUtilsMessengerEXT m_dmessenger{nullptr};
};
} // namespace graphics
} // namespace throttle