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

#include <set>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace throttle {
namespace graphics {

inline bool device_support_extensions(const vk::raii::PhysicalDevice  &p_device,
                                      const std::vector<const char *> &p_required_extensions) {
  std::set<std::string> required_extensions{p_required_extensions.begin(), p_required_extensions.end()};
  for (auto &extension : p_device.enumerateDeviceExtensionProperties()) {
    required_extensions.erase(extension.extensionName);
  }
  return required_extensions.empty();
}

inline bool is_suitable(const vk::raii::PhysicalDevice &p_device) {
  const std::vector<const char *> required_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  return device_support_extensions(p_device, required_extensions);
}

inline vk::raii::PhysicalDevice pick_physical_device(const vk::raii::Instance &p_instance) {
  std::vector<vk::raii::PhysicalDevice> available_devices = p_instance.enumeratePhysicalDevices();
  for (auto &device : available_devices) {
    if (is_suitable(device)) return device;
  }
  return nullptr;
}
} // namespace graphics
} // namespace throttle