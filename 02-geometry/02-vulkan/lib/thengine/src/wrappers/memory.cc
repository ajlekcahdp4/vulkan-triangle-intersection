/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "wrappers/memory.hpp"

namespace throttle::graphics {

uint32_t find_memory_type(const vk::PhysicalDeviceMemoryProperties &p_mem_properties, uint32_t &p_type_filter,
                          const vk::MemoryPropertyFlags &p_property_flags) {
  for (uint32_t i = 0; i < p_mem_properties.memoryTypeCount; ++i)
    if ((p_type_filter & (1 << i)) &&
        ((p_mem_properties.memoryTypes[i].propertyFlags & p_property_flags) == p_property_flags))
      return i;
  throw std::runtime_error("failed to find suitable memory type!");
}

vk::raii::DeviceMemory allocate_device_memory(const vk::raii::Device                  &p_device,
                                              const vk::PhysicalDeviceMemoryProperties p_mem_properties,
                                              vk::MemoryRequirements                   p_mem_requirements,
                                              const vk::MemoryPropertyFlags            p_mem_property_flags) {
  uint32_t mem_type_index = find_memory_type(p_mem_properties, p_mem_requirements.memoryTypeBits, p_mem_property_flags);

  vk::MemoryAllocateInfo mem_allocate_info{.allocationSize = p_mem_requirements.size,
                                           .memoryTypeIndex = mem_type_index};

  return vk::raii::DeviceMemory{p_device, mem_allocate_info};
}

} // namespace throttle::graphics