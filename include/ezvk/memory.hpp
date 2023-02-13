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

#include "ezvk/error.hpp"
#include "ezvk/memory.hpp"

#include "ezvk/queues.hpp"
#include "utils.hpp"

#include "vulkan_hpp_include.hpp"

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace ezvk {

class vk_memory_error : public ezvk::vk_error {
public:
  vk_memory_error(std::string msg) : ezvk::vk_error{msg} {}
};

inline uint32_t find_memory_type(
    vk::PhysicalDeviceMemoryProperties mem_properties, uint32_t &type_filter, vk::MemoryPropertyFlags property_flags) {
  uint32_t i = 0;
  auto found = std::find_if(
      mem_properties.memoryTypes.begin(), mem_properties.memoryTypes.end(), [&i, property_flags, type_filter](auto a) {
        return (type_filter & (1 << i++)) && ((a.propertyFlags & property_flags) == property_flags);
      });

  if (found == mem_properties.memoryTypes.end()) throw ezvk::vk_memory_error{"Could not find suitable memory type"};
  return i - 1;
}

inline vk::raii::DeviceMemory allocate_device_memory(const vk::raii::Device &l_device,
    vk::PhysicalDeviceMemoryProperties properties, vk::MemoryRequirements requirements,
    vk::MemoryPropertyFlags property_flags) {
  uint32_t mem_type_index = find_memory_type(properties, requirements.memoryTypeBits, property_flags);
  vk::MemoryAllocateInfo mem_allocate_info{.allocationSize = requirements.size, .memoryTypeIndex = mem_type_index};

  return {l_device, mem_allocate_info};
}

class framebuffers final {
private:
  std::vector<vk::raii::Framebuffer> m_vector;

public:
  framebuffers() = default;

  framebuffers(const vk::raii::Device &l_device, const std::vector<vk::raii::ImageView> &image_views,
      const vk::Extent2D &extent, const vk::raii::RenderPass &render_pass) {
    uint32_t n_framebuffers = image_views.size();
    m_vector.reserve(n_framebuffers);

    for (const auto &view : image_views) {
      vk::FramebufferCreateInfo framebuffer_info = {.renderPass = *render_pass,
          .attachmentCount = 1,
          .pAttachments = std::addressof(*view),
          .width = extent.width,
          .height = extent.height,
          .layers = 1};
      m_vector.emplace_back(l_device, framebuffer_info);
    }
  }

  framebuffers(const vk::raii::Device &l_device, const std::vector<vk::raii::ImageView> &image_views,
      const vk::Extent2D &extent, const vk::raii::RenderPass &render_pass,
      const vk::raii::ImageView &depth_image_view) {
    uint32_t n_framebuffers = image_views.size();
    m_vector.reserve(n_framebuffers);

    for (const auto &view : image_views) {
      std::array<vk::ImageView, 2> attachments{*view, *depth_image_view};
      vk::FramebufferCreateInfo framebuffer_info = {.renderPass = *render_pass,
          .attachmentCount = attachments.size(),
          .pAttachments = attachments.data(),
          .width = extent.width,
          .height = extent.height,
          .layers = 1};
      m_vector.emplace_back(l_device, framebuffer_info);
    }
  }

  auto &operator[](std::size_t pos) & { return m_vector.at(pos); }
  const auto &operator[](std::size_t pos) const & { return m_vector.at(pos); }

  auto &front() & { return m_vector.front(); }
  const auto &front() const & { return m_vector.front(); }
  auto &back() & { return m_vector.back(); }
  const auto &back() const & { return m_vector.back(); }

  auto size() const { return m_vector.size(); }
  auto begin() { return m_vector.begin(); }
  auto end() { return m_vector.end(); }
  auto begin() const { return m_vector.begin(); }
  auto end() const { return m_vector.end(); }
  auto cbegin() const { return m_vector.cbegin(); }
  auto cend() const { return m_vector.cend(); }
};

class device_buffer final {
  vk::raii::Buffer m_buffer = nullptr;
  vk::raii::DeviceMemory m_memory = nullptr;

public:
  device_buffer() = default;

  // Sure, this is not encapsulation, but rather a consistent interface
  auto &buffer() { return m_buffer; }
  const auto &buffer() const { return m_buffer; }
  auto &memory() { return m_memory; }
  const auto &memory() const { return m_memory; }

  static constexpr auto default_property_flags =
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

  device_buffer(const vk::raii::PhysicalDevice &p_device, const vk::raii::Device &l_device, vk::DeviceSize size,
      vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property_flags = default_property_flags) {
    m_buffer = l_device.createBuffer(vk::BufferCreateInfo{.size = size, .usage = usage});
    m_memory = {allocate_device_memory(
        l_device, p_device.getMemoryProperties(), m_buffer.getMemoryRequirements(), property_flags)};
    m_buffer.bindMemory(*m_memory, 0);
  }

  template <typename T>
  device_buffer(const vk::raii::PhysicalDevice &p_device, const vk::raii::Device &l_device,
      const vk::BufferUsageFlags usage, std::span<const T> data,
      vk::MemoryPropertyFlags property_flags = default_property_flags)
      : device_buffer{p_device, l_device, utils::sizeof_container(data), usage, property_flags} {
    copy_to_device(data);
  }

  template <typename T> void copy_to_device(std::span<const T> view, vk::DeviceSize stride = sizeof(T)) {
    assert(sizeof(T) <= stride);

    auto memory = static_cast<uint8_t *>(m_memory.mapMemory(0, view.size() * stride));

    if (stride == sizeof(T)) {
      std::memcpy(memory, view.data(), view.size() * sizeof(T));
    }

    else {
      for (unsigned i = 0; i < view.size(); ++i) {
        std::memcpy(memory + stride * i, &view[i], sizeof(T));
      }
    }

    m_memory.unmapMemory();
  }

  template <typename T> void copy_to_device(const T &data) { copy_to_device<T>(std::span<const T>{&data, 1}); }
};

class device_buffers final {
  std::vector<device_buffer> m_vector;

public:
  device_buffers() = default;

  device_buffers(std::size_t count, std::size_t max_size, const vk::raii::PhysicalDevice &p_device,
      const vk::raii::Device &l_device, vk::BufferUsageFlags usage,
      vk::MemoryPropertyFlags property_flags = device_buffer::default_property_flags) {
    m_vector.reserve(count);
    for (unsigned i = 0; i < count; i++) {
      m_vector.emplace_back(p_device, l_device, max_size, usage, property_flags);
    }
  }

  auto &operator[](std::size_t pos) & { return m_vector.at(pos); }
  const auto &operator[](std::size_t pos) const & { return m_vector.at(pos); }

  auto &front() & { return m_vector.front(); }
  const auto &front() const & { return m_vector.front(); }
  auto &back() & { return m_vector.back(); }
  const auto &back() const & { return m_vector.back(); }

  auto size() const { return m_vector.size(); }
  auto begin() { return m_vector.begin(); }
  auto end() { return m_vector.end(); }
  auto begin() const { return m_vector.begin(); }
  auto end() const { return m_vector.end(); }
  auto cbegin() const { return m_vector.cbegin(); }
  auto cend() const { return m_vector.cend(); }
};

class upload_context {
  const vk::raii::Device *m_device_ptr;
  const device_queue *m_transfer_queue;

  vk::raii::Fence m_upload_fence = nullptr;
  vk::raii::CommandBuffer m_command_buffer = nullptr;

public:
  upload_context() = default;

  upload_context(
      const vk::raii::Device *l_device, const device_queue *transfer_queue, const vk::raii::CommandPool *pool)
      : m_device_ptr{l_device}, m_transfer_queue{transfer_queue} {
    assert(l_device);
    assert(transfer_queue);

    vk::CommandBufferAllocateInfo alloc_info = {
        .commandPool = **pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};

    m_command_buffer = std::move(vk::raii::CommandBuffers{*m_device_ptr, alloc_info}.front());
    m_upload_fence = l_device->createFence({});
  }

  void immediate_submit(std::function<void(vk::raii::CommandBuffer &cmd)> cmd_buffer_creation_func) {
    m_command_buffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmd_buffer_creation_func(m_command_buffer);
    m_command_buffer.end();

    vk::SubmitInfo submit_info = {.commandBufferCount = 1, .pCommandBuffers = &(*m_command_buffer)};
    m_transfer_queue->queue().submit(submit_info, *m_upload_fence);

    static_cast<void>(m_device_ptr->waitForFences(*m_upload_fence, true, UINT64_MAX));
    m_device_ptr->resetFences(*m_upload_fence);
    m_command_buffer.reset();
  };
};

} // namespace ezvk