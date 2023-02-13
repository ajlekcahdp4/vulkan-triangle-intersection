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

#include "unified_includes/vulkan_hpp_include.hpp"

#include "memory.hpp"

namespace ezvk {

class image final {
  vk::raii::Image m_image = nullptr;
  vk::raii::DeviceMemory m_image_memory = nullptr;

public:
  image() = default;

  image(const vk::raii::PhysicalDevice &p_device, const vk::raii::Device &l_device, const vk::Extent3D &extent,
      const vk::Format format, const vk::ImageTiling tiling, const vk::ImageUsageFlags usage,
      const vk::MemoryPropertyFlags properties) {
    // clang-format off
    vk::ImageCreateInfo image_info = {
        .imageType = vk::ImageType::e2D, .format = format, .extent = extent, .mipLevels = 1,
        .arrayLayers = 1, .samples = vk::SampleCountFlagBits::e1, .tiling = tiling,
        .usage = usage, .sharingMode = vk::SharingMode::eExclusive, .initialLayout = vk::ImageLayout::eUndefined,
    };
    // clang-format on

    m_image = l_device.createImage(image_info);
    auto memory_requirements = (*l_device).getImageMemoryRequirements(*m_image);

    auto phys_device_memory_props = p_device.getMemoryProperties();

    vk::MemoryAllocateInfo alloc_info = {.allocationSize = memory_requirements.size,
        .memoryTypeIndex = find_memory_type(phys_device_memory_props, memory_requirements.memoryTypeBits, properties)};

    m_image_memory = l_device.allocateMemory(alloc_info);
    vk::BindImageMemoryInfo bind_info = {.image = *m_image, .memory = *m_image_memory};
    l_device.bindImageMemory2({bind_info});
  }

  auto &operator()() & { return m_image; }
  const auto &operator()() const & { return m_image; }
};

class image_view final {
  vk::raii::ImageView m_image_view = nullptr;

public:
  image_view() = default;

  image_view(const vk::raii::Device &l_device, vk::raii::Image &image, const vk::Format format,
      const vk::ImageAspectFlagBits aspect_flags) {
    // clang-format off
    vk::ImageSubresourceRange range = {.aspectMask = aspect_flags, .baseMipLevel = 0,
                                       .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

    vk::ImageViewCreateInfo iv_create_info = {
        .image = *image, .viewType = vk::ImageViewType::e2D,
        .format = format, .subresourceRange = range,
    };
    // clang-format on
    m_image_view = l_device.createImageView(iv_create_info);
  }

  auto &operator()() & { return m_image_view; }
  const auto &operator()() const & { return m_image_view; }
};
} // namespace ezvk