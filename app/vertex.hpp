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

#include "vulkan_hpp_include.hpp"
#include <glm/glm.hpp>

#include <array>

namespace triangles {

struct vertex_type {
  glm::vec3 pos;
  glm::vec3 color;

public:
  static constexpr vk::VertexInputBindingDescription get_binding_description() {
    return {.binding = 0, .stride = sizeof(vertex_type), .inputRate = vk::VertexInputRate::eVertex};
  }

  static constexpr std::array<vk::VertexInputAttributeDescription, 2> get_attribute_description() {
    auto first = vk::VertexInputAttributeDescription{
        .location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(vertex_type, pos)};
    auto second = vk::VertexInputAttributeDescription{
        .location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(vertex_type, color)};
    return {first, second};
  }
};

} // namespace triangles