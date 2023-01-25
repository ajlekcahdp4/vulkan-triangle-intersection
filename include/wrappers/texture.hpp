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

#include "images.hpp"
#include "memory.hpp"

namespace throttle::graphics {

struct texture_data {
  texture_data(const vk::raii::PhysicalDevice &p_physical_device, const vk::raii::Device &p_logical_device,
               const vk::Extent2D &p_extent = {256, 256}, vk::ImageUsageFlags p_usage_flags = {},
               vk::FormatFeatureFlags p_format_feature_flags = {}, bool anisotropy_enable = false,
               bool force_staging = false)
      : m_format{vk::Format::eR8G8B8A8Unorm}, m_extent{p_extent},
        m_sampler{p_logical_device, vk::SamplerCreateInfo{.flags = {},
                                                          .magFilter = vk::Filter::eLinear,
                                                          .minFilter = vk::Filter::eLinear,
                                                          .mipmapMode = vk::SamplerMipmapMode::eLinear,
                                                          .mipLodBias = 0.0f,
                                                          .anisotropyEnable = anisotropy_enable,
                                                          .maxAnisotropy = 16.0f,
                                                          .compareEnable = false,
                                                          .compareOp = vk::CompareOp::eNever,
                                                          .minLod = 0.0f,
                                                          .maxLod = 0.0f,
                                                          .borderColor = vk::BorderColor::eFloatOpaqueBlack}} {
    auto format_properties = p_physical_device.getFormatProperties(m_format);
    p_format_feature_flags |= vk::FormatFeatureFlagBits::eSampledImage;
    needs_staging =
        force_staging || ((format_properties.linearTilingFeatures & p_format_feature_flags) != p_format_feature_flags);
    vk::ImageTiling         image_tiling;
    vk::ImageLayout         initial_layout;
    vk::MemoryPropertyFlags requirements;
    if (needs_staging) {
      m_staging_buffer = buffer{p_physical_device, p_logical_device, m_extent.width * m_extent.width * 4,
                                vk::BufferUsageFlagBits::eTransferSrc};
      image_tiling = vk::ImageTiling::eOptimal;
      p_usage_flags |= vk::ImageUsageFlagBits::eTransferSrc;
      initial_layout = vk::ImageLayout::eUndefined;
    } else {
      image_tiling = vk::ImageTiling::eLinear;
      initial_layout = vk::ImageLayout::ePreinitialized;
      requirements = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
    }
    m_image_data = image_data{p_physical_device, p_logical_device, m_format,
                              m_extent,          image_tiling,     p_usage_flags | vk::ImageUsageFlagBits::eSampled,
                              initial_layout,    requirements,     vk::ImageAspectFlagBits::eColor};
  }

  vk::Format        m_format;
  vk::Extent2D      m_extent;
  bool              needs_staging{};
  buffer            m_staging_buffer{nullptr};
  image_data        m_image_data{nullptr};
  vk::raii::Sampler m_sampler;
};

} // namespace throttle::graphics