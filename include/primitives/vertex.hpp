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

  static vk::VertexInputBindingDescription get_binding_description() {
    vk::VertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(vertex);
    binding_description.inputRate = vk::VertexInputRate::eVertex;
    return binding_description;
  }

  static std::array<vk::VertexInputAttributeDescription, 2> get_attribute_description() {
    std::array<vk::VertexInputAttributeDescription, 2> attribute_description{};

    vk::VertexInputAttributeDescription();
    attribute_description[0].binding = 0;
    attribute_description[0].location = 0;
    attribute_description[0].format = vk::Format::eR32G32Sfloat;
    attribute_description[0].offset = offsetof(vertex, pos);

    attribute_description[1].binding = 0;
    attribute_description[1].location = 1;
    attribute_description[1].format = vk::Format::eR32G32B32Sfloat;
    attribute_description[1].offset = offsetof(vertex, color);

    return attribute_description;
  }
};

} // namespace throttle::graphics