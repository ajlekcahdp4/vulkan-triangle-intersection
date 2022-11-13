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
#include <utility>

#include "vulkan_include.hpp"

#include "surface.hpp"

namespace throttle::graphics {

uint32_t                      find_graphics_family_index(const std::vector<vk::QueueFamilyProperties> &);
std::pair<uint32_t, uint32_t> find_graphics_and_present_family_indices(const vk::raii::PhysicalDevice &p_device,
                                                                       const vk::raii::SurfaceKHR     &p_surface);

struct queues {
  vk::raii::Queue graphics = nullptr;
  vk::raii::Queue present = nullptr;

  uint32_t graphics_index = 0;
  uint32_t present_index = 0;

  queues(const vk::raii::PhysicalDevice &p_device, const vk::raii::Device &l_device,
         const vk::raii::SurfaceKHR &p_surface) {
    auto indices = find_graphics_and_present_family_indices(p_device, p_surface);
    graphics_index = indices.first;
    present_index = indices.second;
    graphics = l_device.getQueue(indices.first, 0);
    present = l_device.getQueue(indices.second, 0);
  }
};

} // namespace throttle::graphics