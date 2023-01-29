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

#include "utils.hpp"
#include "vulkan_hpp_include.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ezvk {

using queue_family_index = uint32_t;
struct queue_family_indices {
  queue_family_index graphics, present;
};

inline std::optional<queue_family_index> find_graphics_family_index(auto properties_start, auto properties_finish) {
  auto found = std::find_if(properties_start, properties_finish, [](vk::QueueFamilyProperties const &qfp) {
    return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
  });

  if (found == properties_finish) return std::nullopt;

  return static_cast<queue_family_index>(std::distance(properties_start, found));
}

inline std::optional<queue_family_indices>
find_graphics_and_present_family_indices(const vk::raii::PhysicalDevice &p_device,
                                         const vk::raii::SurfaceKHR     &p_surface) {
  auto queue_family_properties = p_device.getQueueFamilyProperties();
  auto graphics_family_index =
      find_graphics_family_index(queue_family_properties.begin(), queue_family_properties.end());

  if (!graphics_family_index) return std::nullopt;

  for (queue_family_index idx = 0; idx < queue_family_properties.size(); ++idx) {
    if (p_device.getSurfaceSupportKHR(idx, *p_surface)) {
      return queue_family_indices{.graphics = graphics_family_index.value(), .present = idx};
    }
  }

  return std::nullopt;
}

inline vk::raii::CommandPool create_command_pool(const vk::raii::Device &device, const queue_family_indices queues) {
  return device.createCommandPool(vk::CommandPoolCreateInfo{.queueFamilyIndex = queues.graphics});
}

} // namespace ezvk