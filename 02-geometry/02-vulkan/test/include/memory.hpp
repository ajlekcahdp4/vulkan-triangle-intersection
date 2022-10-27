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

#include "pipeline.hpp"
#include "swapchain.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <array>

namespace throttle {
namespace graphics {

std::vector<vk::raii::Framebuffer> allocate_frame_buffers(const vk::raii::Device &p_device,
                                                          i_swapchain_data       &p_swapchain_data,
                                                          i_surface_data         &p_surface_data,
                                                          const pipeline_data    &p_pipeline_data) {
  std::vector<vk::raii::Framebuffer> framebuffers;
  auto                              &image_views = p_swapchain_data.image_views();
  uint32_t                           n_framebuffers = image_views.size();
  framebuffers.reserve(n_framebuffers);
  for (uint32_t i = 0; i < n_framebuffers; i++) {
    vk::ImageView             attachments[] = {*image_views[i]};
    vk::FramebufferCreateInfo framebuffer_info{};
    framebuffer_info.renderPass = *p_pipeline_data.m_render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = p_surface_data.extent().width;
    framebuffer_info.height = p_surface_data.extent().height;
    framebuffer_info.layers = 1;
    framebuffers.push_back(p_device.createFramebuffer(framebuffer_info));
  }
  return framebuffers;
}

} // namespace graphics
} // namespace throttle