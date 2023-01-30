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
    if (supports_extensions(device, ext_start, ext_finish)) suitable_devices.push_back(std::move(device));
  }

  return suitable_devices;
}

}; // namespace physical_device_selector

class logical_device {
  vk::raii::Device m_device = nullptr;

public:
  logical_device() = default;

  logical_device(const vk::raii::PhysicalDevice &p_device, const vk::raii::SurfaceKHR &surface) {
    // TODO[Sergei]: finish refactoring this
  }

  auto       &operator()() { return m_device; }
  const auto &operator()() const { return m_device; }
};

} // namespace ezvk