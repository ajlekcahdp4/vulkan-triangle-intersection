/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "queues.hpp"
#include "utils.hpp"
#include "vulkan_hpp_include.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace ezvk {

namespace physical_device_selector {

using supports_result = std::pair<bool, std::vector<std::string>>;

[[nodiscard]] inline supports_result supports_extensions(const vk::raii::PhysicalDevice &device, auto ext_start,
                                                         auto ext_finish) {
  const auto supported_extensions = device.enumerateDeviceExtensionProperties();
  auto missing_extensions = utils::find_all_missing(supported_extensions.begin(), supported_extensions.end(), ext_start,
                                                    ext_finish, [](auto a) { return a.extensionName; });
  return std::make_pair(missing_extensions.empty(), missing_extensions);
}

inline std::vector<vk::raii::PhysicalDevice> enumerate_suitable_physical_devices(const vk::raii::Instance &instance,
                                                                                 auto ext_start, auto ext_finish) {
  auto                                  available_devices = instance.enumeratePhysicalDevices();
  std::vector<vk::raii::PhysicalDevice> suitable_devices;

  for (const auto &device : available_devices) {
    if (supports_extensions(device, ext_start, ext_finish).first) suitable_devices.push_back(std::move(device));
  }

  return suitable_devices;
}

}; // namespace physical_device_selector

class logical_device {
  vk::raii::Device m_device = nullptr;

public:
  logical_device() = default;

  using device_queue_create_infos = std::vector<vk::DeviceQueueCreateInfo>;
  logical_device(const vk::raii::PhysicalDevice &p_device, device_queue_create_infos requested_queues, auto ext_start,
                 auto ext_finish, vk::PhysicalDeviceFeatures features = {}) {
    // TODO[Sergei]: finish refactoring this
    std::vector<const char *> extensions, layers;
    // For now we ignore device layers completely, this is a dummy vector
    for (; ext_start != ext_finish; ++ext_start) {
      extensions.push_back(ext_start->c_str());
    }

    vk::DeviceCreateInfo device_create_info = {.queueCreateInfoCount = static_cast<uint32_t>(requested_queues.size()),
                                               .pQueueCreateInfos = requested_queues.data(),
                                               .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                                               .ppEnabledLayerNames = layers.data(),
                                               .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                                               .ppEnabledExtensionNames = extensions.data(),
                                               .pEnabledFeatures = &features};

    m_device = p_device.createDevice(device_create_info);
  }

  auto       &operator()() { return m_device; }
  const auto &operator()() const { return m_device; }
};

} // namespace ezvk