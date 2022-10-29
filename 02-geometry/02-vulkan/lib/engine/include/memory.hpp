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
#include "queue_families.hpp"
#include "swapchain.hpp"
#include "vertex.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <array>

namespace throttle {
namespace graphics {

inline std::vector<vk::raii::Framebuffer> allocate_frame_buffers(const vk::raii::Device &p_device,
                                                                 i_swapchain_data       &m_swapchain_data,
                                                                 i_surface_data         &p_surface_data,
                                                                 const pipeline_data    &m_pipeline_data) {
  std::vector<vk::raii::Framebuffer> framebuffers;
  auto                              &image_views = m_swapchain_data.image_views();
  uint32_t                           n_framebuffers = image_views.size();
  framebuffers.reserve(n_framebuffers);
  for (uint32_t i = 0; i < n_framebuffers; i++) {
    vk::ImageView             attachments[] = {*image_views[i]};
    vk::FramebufferCreateInfo framebuffer_info{};
    framebuffer_info.renderPass = *m_pipeline_data.m_render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = p_surface_data.extent().width;
    framebuffer_info.height = p_surface_data.extent().height;
    framebuffer_info.layers = 1;
    framebuffers.push_back(p_device.createFramebuffer(framebuffer_info));
  }
  return framebuffers;
}

inline vk::raii::CommandPool create_command_pool(const vk::raii::Device &p_device, const queues &p_queues) {
  vk::CommandPoolCreateInfo pool_info{};
  pool_info.queueFamilyIndex = p_queues.graphics_index;
  return p_device.createCommandPool(pool_info);
}

inline uint32_t find_memory_type(const vk::PhysicalDeviceMemoryProperties &p_mem_properties, uint32_t &p_type_filter,
                                 const vk::MemoryPropertyFlags &p_property_flags) {
  for (uint32_t i = 0; i < p_mem_properties.memoryTypeCount; ++i)
    if ((p_type_filter & (1 << i)) &&
        ((p_mem_properties.memoryTypes[i].propertyFlags & p_property_flags) == p_property_flags))
      return i;
  throw std::runtime_error("failed to find suitable memory type!");
}

template <typename T> std::size_t sizeof_vector(const std::vector<T> &vec) { return sizeof(T) * vec.size(); }

struct buffer {
  vk::raii::Buffer       m_buffer{nullptr};
  vk::raii::DeviceMemory m_memory{nullptr};

  buffer(std::nullptr_t) {}

  buffer(const vk::raii::PhysicalDevice &p_phys_device, const vk::raii::Device &p_logical_device,
         const vk::DeviceSize p_size, const vk::BufferUsageFlags p_usage,
         vk::MemoryPropertyFlags p_property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                    vk::MemoryPropertyFlagBits::eHostCoherent)
      : m_buffer{p_logical_device.createBuffer(vk::BufferCreateInfo{{}, p_size, p_usage})},
        m_memory{allocate_device_memory(p_logical_device, p_phys_device.getMemoryProperties(),
                                        m_buffer.getMemoryRequirements(), p_property_flags)} {
    m_buffer.bindMemory(*m_memory, 0);
  }

  static vk::raii::DeviceMemory allocate_device_memory(const vk::raii::Device                  &p_device,
                                                       const vk::PhysicalDeviceMemoryProperties p_mem_properties,
                                                       vk::MemoryRequirements                   p_mem_requirements,
                                                       const vk::MemoryPropertyFlags            p_mem_property_flags) {
    uint32_t mem_type_index =
        find_memory_type(p_mem_properties, p_mem_requirements.memoryTypeBits, p_mem_property_flags);
    vk::MemoryAllocateInfo mem_allocate_info{p_mem_requirements.size, mem_type_index};
    return vk::raii::DeviceMemory{p_device, mem_allocate_info};
  }
};

} // namespace graphics
} // namespace throttle