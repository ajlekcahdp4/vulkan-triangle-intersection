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
#include <vector>

namespace throttle {
namespace graphics {

class framebuffers : public std::vector<vk::raii::Framebuffer> {
public:
  framebuffers(const vk::raii::Device &p_device, const std::vector<vk::raii::ImageView> &p_image_views,
               const vk::Extent2D &p_extent, const vk::raii::RenderPass &p_render_pass) {
    uint32_t n_framebuffers = p_image_views.size();
    this->reserve(n_framebuffers);
    for (uint32_t i = 0; i < n_framebuffers; i++) {
      vk::ImageView             attachments[] = {*p_image_views[i]};
      vk::FramebufferCreateInfo framebuffer_info{};
      framebuffer_info.renderPass = *p_render_pass;
      framebuffer_info.attachmentCount = 1;
      framebuffer_info.pAttachments = attachments;
      framebuffer_info.width = p_extent.width;
      framebuffer_info.height = p_extent.height;
      framebuffer_info.layers = 1;
      this->emplace_back(p_device, framebuffer_info);
    }
  }
};

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

  buffer(const vk::raii::PhysicalDevice &p_phys_device, const vk::raii::Device &p_logical_device,
         const vk::DeviceSize p_size, const vk::BufferUsageFlags p_usage, const std::vector<vertex> p_vertices,
         vk::MemoryPropertyFlags p_property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                    vk::MemoryPropertyFlagBits::eHostCoherent)
      : m_buffer{p_logical_device.createBuffer(vk::BufferCreateInfo{{}, p_size, p_usage})},
        m_memory{allocate_device_memory(p_logical_device, p_phys_device.getMemoryProperties(),
                                        m_buffer.getMemoryRequirements(), p_property_flags)} {
    m_buffer.bindMemory(*m_memory, 0);
    char *memory = static_cast<char *>(m_memory.mapMemory(0, p_size));
    memcpy(memory, p_vertices.data(), p_size);
    m_memory.unmapMemory();
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