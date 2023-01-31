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
#include <glm/glm.hpp>

#include <array>

namespace throttle::graphics {

struct vertex {
  glm::vec3 pos;
  glm::vec3 color;

public:
  static constexpr vk::VertexInputBindingDescription get_binding_description() {
    return {.binding = 0, .stride = sizeof(vertex), .inputRate = vk::VertexInputRate::eVertex};
  }

  static constexpr std::array<vk::VertexInputAttributeDescription, 2> get_attribute_description() {
    return std::array<vk::VertexInputAttributeDescription, 2>{
        {.binding = 0, .location = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(vertex, pos)},
        {.binding = 0, .location = 1, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(vertex, color)}};
  }
};

} // namespace throttle::graphics