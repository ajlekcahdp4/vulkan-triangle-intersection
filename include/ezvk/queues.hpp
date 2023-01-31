/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "utils.hpp"
#include "vulkan_hpp_include.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <vulkan/vulkan_raii.hpp>

namespace ezvk {

using queue_family_index = uint32_t;
struct queue_family_indices {
  queue_family_index graphics, present;
};

inline std::vector<queue_family_index> find_family_indices_with_queue_type(const vk::raii::PhysicalDevice &p_device,
                                                                           vk::QueueFlagBits               queue_bits) {
  auto                            properties = p_device.getQueueFamilyProperties();
  std::vector<queue_family_index> graphics_indices;

  for (queue_family_index i = 0; const auto &qfp : properties) {
    if (qfp.queueFlags & queue_bits) graphics_indices.push_back(i);
    ++i;
  }

  return graphics_indices;
}

inline std::vector<queue_family_index> find_graphics_family_indices(const vk::raii::PhysicalDevice &p_device) {
  return find_family_indices_with_queue_type(p_device, vk::QueueFlagBits::eGraphics);
}

inline std::vector<queue_family_index> find_present_family_indices(const vk::raii::PhysicalDevice &p_device,
                                                                   const vk::raii::SurfaceKHR     &surface) {
  auto                            size = p_device.getQueueFamilyProperties().size();
  std::vector<queue_family_index> present_indices;

  for (queue_family_index i = 0; i < size; ++i) {
    if (p_device.getSurfaceSupportKHR(i, *surface)) present_indices.push_back(i);
  }

  return present_indices;
}

class i_graphics_present_queues {
public:
  virtual vk::Queue graphics() const = 0;
  virtual vk::Queue present() const = 0;
  virtual ~i_graphics_present_queues() {}
};

using queue_index = uint32_t;

class device_queue {
  vk::raii::Queue    m_queue = nullptr;
  queue_index        m_queue_index;
  queue_family_index m_queue_family_index;

public:
  device_queue() = default;

  device_queue(const vk::raii::Device &l_device, queue_family_index queue_family, queue_index index) {
    m_queue = l_device.getQueue(queue_family, index);
    m_queue_index = index;
    m_queue_family_index = queue_family;
  }

  auto family_index() const { return m_queue_family_index; }
  auto queue_index() const { return m_queue_index; }

  auto       &queue() { return m_queue; }
  const auto &queue() const { return m_queue; }
};

namespace detail {

class separate_graphics_present_queues : public i_graphics_present_queues {
  vk::raii::Queue m_graphics = nullptr, m_present = nullptr;

public:
  separate_graphics_present_queues(const vk::raii::Device &l_device, queue_family_index graphics_family,
                                   queue_index graphics, queue_family_index present_family, queue_index present) {
    m_graphics = l_device.getQueue(graphics_family, graphics);
    m_present = l_device.getQueue(present_family, present);
  }

  vk::Queue graphics() const override { return *m_graphics; }
  vk::Queue present() const override { return *m_present; }
};

class single_graphics_present_queues : i_graphics_present_queues {
  vk::raii::Queue m_queue = nullptr;

public:
  single_graphics_present_queues(const vk::raii::Device &l_device, queue_family_index family, queue_index index) {
    m_queue = l_device.getQueue(family, index);
  }

  vk::Queue graphics() const override { return *m_queue; }
  vk::Queue present() const override { return *m_queue; }
};

} // namespace detail

inline vk::raii::CommandPool create_command_pool(const vk::raii::Device &device, queue_family_index queue) {
  return device.createCommandPool(vk::CommandPoolCreateInfo{.queueFamilyIndex = queue});
}

} // namespace ezvk