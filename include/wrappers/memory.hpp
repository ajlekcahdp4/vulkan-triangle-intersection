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
#include <vector>

namespace throttle::graphics {

class framebuffers final {
private:
  std::vector<vk::raii::Framebuffer> m_vector;

public:
  framebuffers() {}

  framebuffers(const vk::raii::Device &p_device, const std::vector<vk::raii::ImageView> &p_image_views,
               const vk::Extent2D &p_extent, const vk::raii::RenderPass &p_render_pass) {
    uint32_t n_framebuffers = p_image_views.size();
    m_vector.reserve(n_framebuffers);
    for (uint32_t i = 0; i < n_framebuffers; i++) {
      vk::ImageView             attachments[] = {*p_image_views[i]};
      vk::FramebufferCreateInfo framebuffer_info{};
      framebuffer_info.renderPass = *p_render_pass;
      framebuffer_info.attachmentCount = 1;
      framebuffer_info.pAttachments = attachments;
      framebuffer_info.width = p_extent.width;
      framebuffer_info.height = p_extent.height;
      framebuffer_info.layers = 1;
      m_vector.emplace_back(p_device, framebuffer_info);
    }
  }

  auto &operator[](std::size_t pos) { return m_vector[pos]; }

  auto size() { return m_vector.size(); }

  auto begin() { return m_vector.begin(); }

  auto end() { return m_vector.end(); }
};

uint32_t find_memory_type(const vk::PhysicalDeviceMemoryProperties &p_mem_properties, uint32_t &p_type_filter,
                          const vk::MemoryPropertyFlags &p_property_flags);

vk::raii::DeviceMemory allocate_device_memory(const vk::raii::Device                  &p_device,
                                              const vk::PhysicalDeviceMemoryProperties p_mem_properties,
                                              vk::MemoryRequirements                   p_mem_requirements,
                                              const vk::MemoryPropertyFlags            p_mem_property_flags);

template <typename T> std::size_t sizeof_vector(const std::vector<T> &vec) { return sizeof(T) * vec.size(); }

struct buffer final {
  vk::raii::Buffer       m_buffer{nullptr};
  vk::raii::DeviceMemory m_memory{nullptr};

  buffer(std::nullptr_t) {}

  buffer(const vk::raii::PhysicalDevice &p_phys_device, const vk::raii::Device &p_logical_device,
         const vk::DeviceSize p_size, const vk::BufferUsageFlags p_usage,
         vk::MemoryPropertyFlags p_property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                    vk::MemoryPropertyFlagBits::eHostCoherent)
      : m_buffer{p_logical_device.createBuffer(vk::BufferCreateInfo{.size = p_size, .usage = p_usage})},
        m_memory{allocate_device_memory(p_logical_device, p_phys_device.getMemoryProperties(),
                                        m_buffer.getMemoryRequirements(), p_property_flags)} {
    m_buffer.bindMemory(*m_memory, 0);
  }

  template <typename T>
  buffer(const vk::raii::PhysicalDevice &p_phys_device, const vk::raii::Device &p_logical_device,
         const vk::BufferUsageFlags p_usage, const std::vector<T> &p_data,
         vk::MemoryPropertyFlags p_property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                    vk::MemoryPropertyFlagBits::eHostCoherent)
      : m_buffer{p_logical_device.createBuffer(vk::BufferCreateInfo{.size = sizeof_vector(p_data), .usage = p_usage})},
        m_memory{allocate_device_memory(p_logical_device, p_phys_device.getMemoryProperties(),
                                        m_buffer.getMemoryRequirements(), p_property_flags)} {
    m_buffer.bindMemory(*m_memory, 0);
    copy_to_device(p_data.data(), p_data.size());
  }

  template <typename T> void copy_to_device(const T *data, const std::size_t count, vk::DeviceSize stride = sizeof(T)) {
    assert(sizeof(T) <= stride);
    auto memory = static_cast<uint8_t *>(m_memory.mapMemory(0, count * stride));
    if (stride == sizeof(T))
      memcpy(memory, data, count * sizeof(T));
    else
      for (unsigned i = 0; i < count; ++i)
        memcpy(memory + stride * i, &data[i], sizeof(T));
    m_memory.unmapMemory();
  }

  template <typename T> void copy_to_device(const T &data) { copy_to_device<T>(&data, 1); }
};

class buffers final {

private:
  std::vector<buffer> m_vector;

public:
  buffers() = default;

  buffers(const std::size_t count, const std::size_t max_size, const vk::raii::PhysicalDevice &p_phys_device,
          const vk::raii::Device &p_logical_device, const vk::BufferUsageFlags p_usage,
          vk::MemoryPropertyFlags p_property_flags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent) {
    m_vector.reserve(count);
    for (unsigned i = 0; i < count; i++) {
      m_vector.emplace_back(p_phys_device, p_logical_device, max_size, p_usage, p_property_flags);
    }
  }

  auto &operator[](std::size_t pos) { return m_vector[pos]; }

  auto size() { return m_vector.size(); }

  auto begin() { return m_vector.begin(); }

  auto end() { return m_vector.end(); }
};

} // namespace throttle::graphics