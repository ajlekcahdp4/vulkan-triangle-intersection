/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <string_view>

#include "vulkan_include.hpp"

#include "wrappers/device.hpp"
#include "wrappers/queue_families.hpp"
#include "wrappers/surface.hpp"

namespace throttle::graphics {

using vk::raii::PhysicalDevice;

bool device_supports_extensions(const PhysicalDevice &p_device, const std::vector<const char *> &p_req_ext) {
  std::set<std::string> required_extensions{p_req_ext.begin(), p_req_ext.end()};

  for (const auto &extension : p_device.enumerateDeviceExtensionProperties()) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

bool is_device_suitable(const vk::raii::PhysicalDevice &p_device) {
  const std::vector<const char *> required_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  return device_supports_extensions(p_device, required_extensions);
}

vk::raii::PhysicalDevice pick_physical_device(const vk::raii::Instance &p_inst) {
  auto available_devices = p_inst.enumeratePhysicalDevices();

  for (auto &device : available_devices) {
    if (is_device_suitable(device)) return device;
  }

  return nullptr;
}

vk::raii::Device create_device(const vk::raii::PhysicalDevice &p_device, const vk::raii::SurfaceKHR &p_surface) {
  const auto indices = find_graphics_and_present_family_indices(p_device, p_surface);

  std::set<uint32_t> unique_indices;
  unique_indices.insert(indices.first);
  unique_indices.insert(indices.second);

  float queue_priority = 1.0f;

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

  for (auto qfi : unique_indices) {
    vk::DeviceQueueCreateInfo queue_create_info = {
        .queueFamilyIndex = qfi, .queueCount = 1, .pQueuePriorities = &queue_priority};
    queue_create_infos.push_back(queue_create_info);
  }

  std::vector<const char *>  device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::PhysicalDeviceFeatures device_features;
  std::vector<const char *>  enabled_layers;

  vk::DeviceCreateInfo device_create_info = {.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
                                             .pQueueCreateInfos = queue_create_infos.data(),
                                             .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
                                             .ppEnabledLayerNames = enabled_layers.data(),
                                             .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
                                             .ppEnabledExtensionNames = device_extensions.data(),
                                             .pEnabledFeatures = &device_features};

  return p_device.createDevice(device_create_info);
}

}; // namespace throttle::graphics