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

#include "queue_families.hpp"
#include "surface.hpp"
#include <cstddef>

namespace throttle::graphics {

class swapchain_wrapper final {
private:
  vk::SurfaceFormatKHR             m_format;
  vk::Extent2D                     m_extent;
  vk::raii::SwapchainKHR           m_handle = nullptr;
  std::vector<vk::Image>           m_images;
  std::vector<vk::raii::ImageView> m_image_views;

public:
  swapchain_wrapper(std::nullptr_t) {}

  swapchain_wrapper(const vk::raii::PhysicalDevice &p_phys_device, const vk::raii::Device &p_logical_device,
                    const vk::raii::SurfaceKHR &p_surface, const vk::Extent2D &p_extent) {
    auto capabilities = p_phys_device.getSurfaceCapabilitiesKHR(*p_surface);
    auto formats = p_phys_device.getSurfaceFormatsKHR(*p_surface);
    auto present_modes = p_phys_device.getSurfacePresentModesKHR(*p_surface);
    auto present_mode = choose_swapchain_present_mode(present_modes);
    m_extent = choose_swapchain_extent(p_extent, capabilities);
    m_format = choose_swapchain_surface_format(formats);
    uint32_t                   image_count = std::max(capabilities.maxImageCount, capabilities.minImageCount + 1);
    vk::SwapchainCreateInfoKHR create_info = {.surface = *p_surface,
                                              .minImageCount = image_count,
                                              .imageFormat = m_format.format,
                                              .imageColorSpace = m_format.colorSpace,
                                              .imageExtent = m_extent,
                                              .imageArrayLayers = 1,
                                              .imageUsage = vk::ImageUsageFlagBits::eColorAttachment};
    auto     queue_family_indices = find_graphics_and_present_family_indices(p_phys_device, p_surface);
    uint32_t arr_indices[2] = {queue_family_indices.first, queue_family_indices.second};
    if (queue_family_indices.first != queue_family_indices.second) {
      create_info.imageSharingMode = vk::SharingMode::eConcurrent;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices = arr_indices;
    } else {
      create_info.imageSharingMode = vk::SharingMode::eExclusive; // TODO[Sergei]: Is this right?
    }
    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    m_handle = p_logical_device.createSwapchainKHR(create_info);
    m_images = (*p_logical_device).getSwapchainImagesKHR(*m_handle);
    m_image_views.reserve(m_images.size());

    for (auto &image : m_images) {
      vk::ImageSubresourceRange subrange_info = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                                          .baseMipLevel = 0,
                                                                          .levelCount = 1,
                                                                          .baseArrayLayer = 0,
                                                                          .layerCount = 1};

      vk::ComponentMapping component_mapping = vk::ComponentMapping{.r = vk::ComponentSwizzle::eIdentity,
                                                                    .g = vk::ComponentSwizzle::eIdentity,
                                                                    .b = vk::ComponentSwizzle::eIdentity,
                                                                    .a = vk::ComponentSwizzle::eIdentity};

      vk::ImageViewCreateInfo image_view_create_info = {.image = image,
                                                        .viewType = vk::ImageViewType::e2D,
                                                        .format = create_info.imageFormat,
                                                        .components = component_mapping,
                                                        .subresourceRange = subrange_info};

      m_image_views.push_back(p_logical_device.createImageView(image_view_create_info));
    }
  }

  vk::raii::SwapchainKHR           &swapchain() { return m_handle; }
  std::vector<vk::Image>           &images() { return m_images; }
  std::vector<vk::raii::ImageView> &image_views() { return m_image_views; }
  vk::SurfaceFormatKHR             &format() { return m_format; }
  vk::Extent2D                     &extent() { return m_extent; }

private:
  static vk::SurfaceFormatKHR choose_swapchain_surface_format(const std::vector<vk::SurfaceFormatKHR> &formats);

  static vk::PresentModeKHR choose_swapchain_present_mode(const std::vector<vk::PresentModeKHR> &present_modes);

  static vk::Extent2D choose_swapchain_extent(const vk::Extent2D &p_extent, const vk::SurfaceCapabilitiesKHR &p_cap);
};

} // namespace throttle::graphics