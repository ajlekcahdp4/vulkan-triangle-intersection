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

#include "vulkan_include.hpp"

namespace throttle {
namespace graphics {

bool device_supports_extensions(const vk::raii::PhysicalDevice &, const std::vector<const char *> &);
bool is_device_suitable(const vk::raii::PhysicalDevice &);

vk::raii::PhysicalDevice pick_physical_device(const vk::raii::Instance &);
vk::raii::Device         create_device(const vk::raii::PhysicalDevice &p_device, const vk::raii::SurfaceKHR &p_surface);

} // namespace graphics
} // namespace throttle