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

#include <iostream>
#include <set>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "queue_families.hpp"
#include "surface.hpp"

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
  auto available_devices = p_instance.enumeratePhysicalDevices();
  for (auto &device : available_devices) {
    if (is_suitable(device)) return device;
  }
  return nullptr;
}

inline vk::raii::Device create_device(const vk::raii::PhysicalDevice &p_device, const vk::raii::SurfaceKHR &p_surface) {
  auto               indices = find_graphics_and_present_family_indices(p_device, p_surface);
  std::set<uint32_t> unique_indices;
  unique_indices.insert(indices.first);
  unique_indices.insert(indices.second);
  float                                  queue_priority = 1.0f;
  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
  for (auto qfi : unique_indices)
    queue_create_infos.push_back(vk::DeviceQueueCreateInfo{vk::DeviceQueueCreateFlags{}, qfi, 1, &queue_priority});
  std::vector<const char *>  device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::PhysicalDeviceFeatures device_features{};
  std::vector<const char *>  enabled_layers;
  // clang-format off
  vk::DeviceCreateInfo device_create_info {
      vk::DeviceCreateFlags {},
      static_cast<uint32_t> (queue_create_infos.size ()),                                    
      queue_create_infos.data (),
      static_cast<uint32_t> (enabled_layers.size ()),
      enabled_layers.data (),
      static_cast<uint32_t>(device_extensions.size()),
      device_extensions.data(),
      &device_features
  };
  // clang-format on
  return p_device.createDevice(device_create_info);
}
} // namespace graphics
} // namespace throttle