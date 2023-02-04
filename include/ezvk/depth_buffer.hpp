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

#include "vulkan_hpp_include.hpp"

#include "image.hpp"
#include "memory.hpp"
#include "utils.hpp"
namespace ezvk {

struct depth_buffer final {
public:
  image      m_image;
  image_view m_image_view;

  depth_buffer() = default;

  depth_buffer(const vk::raii::PhysicalDevice &p_device, const vk::raii::Device &l_device,
               const vk::Extent2D &extent2d) {
    auto depth_format = find_depth_format(p_device);
    m_image = {p_device,
               l_device,
               vk::Extent3D{extent2d, 1},
               depth_format,
               vk::ImageTiling::eOptimal,
               vk::ImageUsageFlagBits::eDepthStencilAttachment,
               vk::MemoryPropertyFlagBits::eDeviceLocal};

    m_image_view = {l_device, m_image(), depth_format};
  }

  static vk::Format find_depth_format(const vk::raii::PhysicalDevice &p_device) {
    auto predicate = [&p_device](const auto &candidate) -> bool {
      auto props = p_device.getFormatProperties(candidate);
      if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) ==
          vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        return true;
      return false;
    };
    std::vector<vk::Format> candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                          VK_FORMAT_D24_UNORM_S8_UINT};

    return utils::find_all_that_satisfy(candidates.begin(), candidates.end(), predicate,
                                        vk::ImageAspectFlagBits::eDepth)
        .front();
  }
}; // namespace ezvk
} // namespace ezvk