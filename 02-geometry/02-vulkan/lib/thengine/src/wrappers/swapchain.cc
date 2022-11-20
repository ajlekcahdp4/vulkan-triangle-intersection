/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "vulkan_include.hpp"

#include "wrappers/swapchain.hpp"

namespace throttle::graphics {

vk::SurfaceFormatKHR
swapchain_wrapper::choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats) {

  auto format_it = std::find_if(formats.begin(), formats.end(), [](auto &format) {
    return (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
  });
  if (format_it != formats.end()) return *format_it;
  return formats.front();
}

vk::PresentModeKHR
swapchain_wrapper::choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR> &present_modes) {

  auto present_mode_it = std::find_if(present_modes.begin(), present_modes.end(),
                                      [](auto &mode) { return mode == vk::PresentModeKHR::eMailbox; });
  if (present_mode_it != present_modes.end()) return *present_mode_it;
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D swapchain_wrapper::choose_swapchain_extent(const vk::Extent2D               &p_extent,
                                                        const vk::SurfaceCapabilitiesKHR &p_cap) {
  // In some systems UINT32_MAX is a flag to say that the size has not been specified
  if (p_cap.currentExtent.width != UINT32_MAX) return p_cap.currentExtent;
  vk::Extent2D extent = {std::min(p_cap.maxImageExtent.width, std::max(p_cap.minImageExtent.width, p_extent.width)),
                         std::min(p_cap.maxImageExtent.height, std::max(p_cap.minImageExtent.height, p_extent.height))};
  return extent;
}

} // namespace throttle::graphics