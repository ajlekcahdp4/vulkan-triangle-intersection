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

#include <vulkan/vulkan_raii.hpp>

#include "queue_families.hpp"
#include "surface.hpp"

namespace throttle {
namespace graphics {

struct i_swapchain_data {
  virtual vk::raii::SwapchainKHR           &swapchain() = 0;
  virtual std::vector<vk::Image>           &images() = 0;
  virtual std::vector<vk::raii::ImageView> &image_views() = 0;
  virtual vk::SurfaceFormatKHR             &format() = 0;
  virtual ~i_swapchain_data() {}
};

class swapchain_data final : public i_swapchain_data {
public:
  swapchain_data(const vk::raii::PhysicalDevice &p_phys_device, const vk::raii::Device &p_logical_device,
                 i_surface_data &p_surface_data, const vk::Extent2D &p_extent) {
    auto capabilities = p_phys_device.getSurfaceCapabilitiesKHR(*p_surface_data.surface());
    auto formats = p_phys_device.getSurfaceFormatsKHR(*p_surface_data.surface());
    auto present_modes = p_phys_device.getSurfacePresentModesKHR(*p_surface_data.surface());
    auto present_mode = choose_swapchain_present_mode(present_modes);
    m_extent = choose_swapchain_extent(p_extent, capabilities);
    m_format = choose_swapchain_surface_format(formats);
    uint32_t image_count = std::max(capabilities.maxImageCount, capabilities.minImageCount + 1);
    // clang-format off
    vk::SwapchainCreateInfoKHR create_info{
      vk::SwapchainCreateFlagsKHR(),
      *p_surface_data.surface(),
      image_count,
      m_format.format,
      m_format.colorSpace,
      m_extent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment
    };
    // clang-format on
    auto     queue_family_indices = find_graphics_and_present_family_indices(p_phys_device, p_surface_data);
    uint32_t arr_indices[2] = {queue_family_indices.first, queue_family_indices.second};
    if (queue_family_indices.first != queue_family_indices.second) {
      create_info.imageSharingMode = vk::SharingMode::eConcurrent;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices = arr_indices;
    } else
      create_info.imageSharingMode = vk::SharingMode::eExclusive;
    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    m_handle = p_logical_device.createSwapchainKHR(create_info);
    m_images = (*p_logical_device).getSwapchainImagesKHR(*m_handle);
    m_image_views.reserve(m_images.size());
    for (auto &image : m_images) {
      vk::ImageViewCreateInfo image_view_create_info = {};
      image_view_create_info.image = image;
      image_view_create_info.viewType = vk::ImageViewType::e2D;
      image_view_create_info.components.r = vk::ComponentSwizzle::eIdentity;
      image_view_create_info.components.g = vk::ComponentSwizzle::eIdentity;
      image_view_create_info.components.b = vk::ComponentSwizzle::eIdentity;
      image_view_create_info.components.a = vk::ComponentSwizzle::eIdentity;
      image_view_create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
      image_view_create_info.subresourceRange.baseMipLevel = 0;
      image_view_create_info.subresourceRange.levelCount = 1;
      image_view_create_info.subresourceRange.baseArrayLayer = 0;
      image_view_create_info.subresourceRange.layerCount = 1;
      image_view_create_info.format = create_info.imageFormat;
      m_image_views.push_back(p_logical_device.createImageView(image_view_create_info));
    }
  }

  vk::raii::SwapchainKHR           &swapchain() override { return m_handle; }
  std::vector<vk::Image>           &images() override { return m_images; }
  std::vector<vk::raii::ImageView> &image_views() override { return m_image_views; }
  vk::SurfaceFormatKHR             &format() override { return m_format; }

private:
  static vk::SurfaceFormatKHR choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats) {
    for (auto &format : formats)
      if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        return format;
    return formats[0];
  }

  static vk::PresentModeKHR choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR> &present_modes) {
    for (auto &mode : present_modes)
      if (mode == vk::PresentModeKHR::eMailbox) // The fastest one
        return mode;
    return vk::PresentModeKHR::eFifo;
  }

  static vk::Extent2D choose_swapchain_extent(const vk::Extent2D               &p_extent,
                                              const vk::SurfaceCapabilitiesKHR &capabilities) {
    // In some systems UINT32_MAX is a flag to say that the size has not been specified
    if (capabilities.currentExtent.width != UINT32_MAX)
      return capabilities.currentExtent;
    else {
      vk::Extent2D extent;

      extent.width =
          std::min(capabilities.maxImageExtent.width, std::max(capabilities.minImageExtent.width, p_extent.width));

      extent.height =
          std::min(capabilities.maxImageExtent.height, std::max(capabilities.minImageExtent.height, p_extent.height));
      return extent;
    }
  }

  vk::SurfaceFormatKHR             m_format;
  vk::Extent2D                     m_extent;
  vk::raii::SwapchainKHR           m_handle{nullptr};
  std::vector<vk::Image>           m_images;
  std::vector<vk::raii::ImageView> m_image_views;
};
} // namespace graphics
} // namespace throttle