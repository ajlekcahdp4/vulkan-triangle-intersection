/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "wrappers/instance.hpp"
#include "vulkan_include.hpp"
#include <string_view>

namespace throttle::graphics {

using vk::raii::Context;
using vk::raii::Instance;

std::vector<const char *> get_required_extensions() {
  uint32_t ext_count = 0;
  auto     arr_glfw_extensions = glfwGetRequiredInstanceExtensions(&ext_count);

  std::vector<const char *> extensions{arr_glfw_extensions, arr_glfw_extensions + ext_count};
  extensions.push_back("VK_EXT_debug_utils");

  return extensions;
}

bool extensions_supported(const std::vector<const char *> &extensions, const Context &context) {
  auto supported_extensions = context.enumerateInstanceExtensionProperties();
  bool found = false;

  for (auto extension : extensions) {
    found = false;

    for (const auto &s_extension : supported_extensions) {
      if (std::strcmp(extension, s_extension.extensionName) == 0) found = true;
    }

    if (!found) return false;
  }

  return true;
}

bool layers_supported(const std::vector<const char *> &layers) {
  auto supported_layers = vk::enumerateInstanceLayerProperties();
  bool found = false;

  for (const auto &layer : layers) {
    found = false;

    for (auto &s_layer : supported_layers) {
      if (std::strcmp(layer, s_layer.layerName) == 0) found = true;
    }

    if (!found) return false;
  }

  return true;
}

// check if we can support all the layer and extensions in current context.
bool is_supported(const std::vector<const char *> &ext, const std::vector<const char *> &layers, const Context &ctx) {
  return extensions_supported(ext, ctx) && layers_supported(layers);
}

vk::raii::Instance instance_wrapper::create_instance() {
  glfwInit();

  Context context;
  auto    version = context.enumerateInstanceVersion();

  vk::ApplicationInfo app_info = {.pApplicationName = "Triangles intersection",
                                  .applicationVersion = version,
                                  .pEngineName = "Best engine",
                                  .engineVersion = version,
                                  .apiVersion = version};

  auto extensions = get_required_extensions();
  auto layers = get_required_layers();

  if (!is_supported(extensions, layers, context)) {
    throw std::runtime_error{"Not all the requested extensions are supported"};
  }

  vk::InstanceCreateInfo create_info = {.pApplicationInfo = &app_info,
                                        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                                        .ppEnabledLayerNames = layers.data(),
                                        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                                        .ppEnabledExtensionNames = extensions.data()};

  return Instance{context, create_info};
}

}; // namespace throttle::graphics