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

#include "memory.hpp"

namespace ezvk {

class image final {
  vk::raii::Image        m_image = nullptr;
  vk::raii::DeviceMemory m_image_memory = nullptr;

public:
  image() = default;

  image(const vk::raii::PhysicalDevice &p_device, const vk::raii::Device &l_device, const vk::Extent3D &extent,
        const vk::Format format, const vk::ImageTiling tiling, const vk::ImageUsageFlags usage,
        const vk::MemoryPropertyFlags properties) {
    vk::ImageCreateInfo image_info = {.imageType = vk::ImageType::e2D,
                                      .extent = extent,
                                      .mipLevels = 1,
                                      .arrayLayers = 1,
                                      .format = format,
                                      .tiling = tiling,
                                      .initialLayout = vk::ImageLayout::eUndefined,
                                      .usage = usage,
                                      .samples = vk::SampleCountFlagBits::e1,
                                      .sharingMode = vk::SharingMode::eExclusive};
    m_image = l_device.createImage(image_info);
    auto memory_requirements = l_device.getImageMemoryRequirements(m_image);

    auto                   phys_device_memory_props = p_device.getMemoryProperties();
    vk::MemoryAllocateInfo alloc_info = {
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex = find_memory_type(phys_device_memory_props, memory_requirements.memoryTypeBits, properties);
    l_device.allocateMemory(alloc_info);
    vk::BindImageMemoryInfo bind_info = {.image = *m_image, .memory = *m_image_memory};
    l_device.bindImageMemory2({bind_info});
  }

  auto       &operator()()       &{ return m_image; }
  const auto &operator()() const & { return m_image; }
};

class image_view final {
  vk::raii::ImageView m_image_view = nullptr;

public:
  image_view() = default;

  image_view(const vk::raii::Device &l_device, vk::raii::Image &image, const vk::Format format,
             const vk::ImageAspectFlagBits aspect_flags) {
    vk::ImageViewCreateInfo iv_create_info = {.image = *image,
                                              .viewType = vk::ImageViewType::e2D,
                                              .format = format,
                                              .subresourceRange.aspectMask = aspect_flags,
                                              .subresourceRange.baseMipLevel = 0,
                                              .subresourceRange.levelCount = 1,
                                              .subresourceRange.baseArrayLayer = 0,
                                              .subresourceRange.layerCount = 1};
    m_image_view = l_device.createImageView(iv_create_info);
  }

  auto       &operator()()       &{ return m_image_view; }
  const auto &operator()() const & { return m_image_view; }
};
} // namespace ezvk