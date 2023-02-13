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

#include "glm_inlcude.hpp"
#include "vulkan_hpp_include.hpp"

#include <array>

namespace triangles {

struct triangle_vertex_type {
  glm::vec3 pos;
  glm::vec3 norm;

  uint32_t color_index;

public:
  static constexpr vk::VertexInputBindingDescription get_binding_description() {
    return {.binding = 0, .stride = sizeof(triangle_vertex_type), .inputRate = vk::VertexInputRate::eVertex};
  }

  static constexpr std::array<vk::VertexInputAttributeDescription, 3> get_attribute_description() {
    auto first = vk::VertexInputAttributeDescription{.location = 0,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(triangle_vertex_type, pos)};

    auto second = vk::VertexInputAttributeDescription{.location = 1,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(triangle_vertex_type, norm)};

    auto third = vk::VertexInputAttributeDescription{.location = 2,
        .binding = 0,
        .format = vk::Format::eR32Uint,
        .offset = offsetof(triangle_vertex_type, color_index)};

    return {first, second, third};
  }
};

struct wireframe_vertex_type {
  glm::vec3 pos;
  uint32_t color_index;

public:
  static constexpr vk::VertexInputBindingDescription get_binding_description() {
    return {.binding = 0, .stride = sizeof(wireframe_vertex_type), .inputRate = vk::VertexInputRate::eVertex};
  }

  static constexpr std::array<vk::VertexInputAttributeDescription, 2> get_attribute_description() {
    auto first = vk::VertexInputAttributeDescription{.location = 0,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(wireframe_vertex_type, pos)};

    auto second = vk::VertexInputAttributeDescription{.location = 1,
        .binding = 0,
        .format = vk::Format::eR32Uint,
        .offset = offsetof(wireframe_vertex_type, color_index)};

    return {first, second};
  }
};

} // namespace triangles