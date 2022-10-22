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

#include <optional>
#include <vulkan/vulkan_raii.hpp>
namespace throttle {
namespace graphics {
struct queue_family_indices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool                    complete() { return graphics_family.has_value() && present_family.has_value(); }
};

inline queue_family_indices find_queue_families(const vk::raii::PhysicalDevice &p_device,
                                                const vk::raii::SurfaceKHR     &p_surface) {
  queue_family_indices indices;
  auto                 queue_families = p_device.getQueueFamilyProperties();
  int                  i = 0;
  for (auto &family : queue_families) {
    if (family.queueFlags & vk::QueueFlagBits::eGraphics) indices.graphics_family = i;
    if (p_device.getSurfaceSupportKHR(i, *p_surface)) indices.present_family = i;
    if (indices.complete()) return indices;
    ++i;
  }
  throw std::runtime_error("Not all required queue families are  supported by physical device");
}

inline std::vector<vk::raii::Queue> get_queue(const vk::raii::PhysicalDevice &p_device,
                                              const vk::raii::Device &l_device, vk::raii::SurfaceKHR &surface) {
  auto inndices = find_queue_families(p_device, surface);
  return {l_device.getQueue(indices.graphics_family.value(), 0), l_device.getQueue(indices.present_family.value(), 0)};
}

} // namespace graphics
} // namespace throttle