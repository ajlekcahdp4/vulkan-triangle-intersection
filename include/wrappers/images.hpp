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

#include "vulkan_include.hpp"

#include "memory.hpp"

namespace throttle::graphics {

struct image_data {

  image_data(const vk::raii::PhysicalDevice &p_physical_device, const vk::raii::Device &p_logical_device,
             vk::Format p_format, const vk::Extent2D &p_extent, vk::ImageTiling p_tiling, vk::ImageUsageFlags p_usage,
             vk::ImageLayout p_initial_layout, vk::MemoryPropertyFlags p_memory_properties,
             vk::ImageAspectFlags p_aspect_mask)
      : m_format{p_format}, m_image{p_logical_device,
                                    vk::ImageCreateInfo{.flags = vk::ImageCreateFlags(),
                                                        .imageType = vk::ImageType::e2D,
                                                        .format = m_format,
                                                        .extent = vk::Extent3D{.width = p_extent.width,
                                                                               .height = p_extent.height,
                                                                               .depth = 1},
                                                        .mipLevels = 1,
                                                        .arrayLayers = 1,
                                                        .tiling = p_tiling,
                                                        .usage = p_usage | vk::ImageUsageFlagBits::eSampled,
                                                        .initialLayout = p_initial_layout}},
        m_device_memory{allocate_device_memory(p_logical_device, p_physical_device.getMemoryProperties(),
                                               m_image.getMemoryRequirements(), p_memory_properties)} {
    m_image.bindMemory(*m_device_memory, 0);
    m_image_view =
        vk::raii::ImageView{p_logical_device, vk::ImageViewCreateInfo{.flags = vk::ImageViewCreateFlags(),
                                                                      .image = *m_image,
                                                                      .viewType = vk::ImageViewType::e2D,
                                                                      .format = m_format,
                                                                      .subresourceRange = {p_aspect_mask, 0, 1, 0, 1}}};
  }

  image_data(std::nullptr_t) {}

  vk::Format             m_format;
  vk::raii::Image        m_image{nullptr};
  vk::raii::ImageView    m_image_view{nullptr};
  vk::raii::DeviceMemory m_device_memory{nullptr};
};
} // namespace throttle::graphics