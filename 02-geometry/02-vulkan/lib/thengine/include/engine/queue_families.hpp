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

#include "surface.hpp"

namespace throttle {
namespace graphics {

inline uint32_t find_graphics_family_index(const std::vector<vk::QueueFamilyProperties> &p_queue_family_properties) {
  std::vector<vk::QueueFamilyProperties>::const_iterator graphics_family_property_it =
      std::find_if(p_queue_family_properties.begin(), p_queue_family_properties.end(),
                   [](vk::QueueFamilyProperties const &qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
  if (graphics_family_property_it == p_queue_family_properties.end())
    throw std::runtime_error("Device does not support all the required queue family indices");
  return static_cast<uint32_t>(std::distance(p_queue_family_properties.begin(), graphics_family_property_it));
}

inline std::pair<uint32_t, uint32_t> find_graphics_and_present_family_indices(const vk::raii::PhysicalDevice &p_device,
                                                                              const vk::raii::SurfaceKHR &p_surface) {
  auto queue_family_properties = p_device.getQueueFamilyProperties();
  auto graphic_family_index = find_graphics_family_index(queue_family_properties);
  for (uint32_t idx = 0; idx < queue_family_properties.size(); ++idx)
    if (p_device.getSurfaceSupportKHR(idx, *p_surface)) return std::make_pair(graphic_family_index, idx);
  throw std::runtime_error("Device does not support all the required queue family indices");
}

struct queues {
  vk::raii::Queue graphics{nullptr};
  vk::raii::Queue present{nullptr};
  uint32_t        graphics_index{};
  uint32_t        present_index{};

  queues(const vk::raii::PhysicalDevice &p_device, const vk::raii::Device &l_device,
         const vk::raii::SurfaceKHR &p_surface) {
    auto indices = find_graphics_and_present_family_indices(p_device, p_surface);
    graphics_index = indices.first;
    present_index = indices.second;
    graphics = l_device.getQueue(indices.first, 0);
    present = l_device.getQueue(indices.second, 0);
  }
};

} // namespace graphics
} // namespace throttle