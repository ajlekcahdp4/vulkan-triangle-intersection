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

class render_pass final {
  vk::raii::RenderPass m_render_pass = nullptr;

public:
  render_pass() = default;

  static constexpr vk::AttachmentDescription default_color_attachment = {
      .flags = vk::AttachmentDescriptionFlags(),
      .format = vk::Format::eB8G8R8A8Unorm,
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::ePresentSrcKHR};

  render_pass(const vk::raii::Device &device, vk::AttachmentDescription color_attachment = default_color_attachment,
              std::span<const vk::SubpassDependency> deps = {}) {

    vk::AttachmentReference color_attachment_ref = {.attachment = 0,
                                                    .layout = vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass = {.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                      .colorAttachmentCount = 1,
                                      .pColorAttachments = &color_attachment_ref};

    vk::RenderPassCreateInfo renderpass_info = {.attachmentCount = 1,
                                                .pAttachments = &color_attachment,
                                                .subpassCount = 1,
                                                .pSubpasses = &subpass,
                                                .dependencyCount = static_cast<uint32_t>(deps.size()),
                                                .pDependencies = deps.data()};

    m_render_pass = device.createRenderPass(renderpass_info);
  }

  auto       &operator()()       &{ return m_render_pass; }
  const auto &operator()() const & { return m_render_pass; }
};

class pipeline_layout final {
  vk::raii::PipelineLayout m_layout = nullptr;

public:
  pipeline_layout() = default;

  pipeline_layout(const vk::raii::Device &device, const vk::raii::DescriptorSetLayout &descriptor_set_layout) {
    vk::PipelineLayoutCreateInfo layout_info = {.flags = vk::PipelineLayoutCreateFlags{},
                                                .setLayoutCount = 1,
                                                .pSetLayouts = std::addressof(*descriptor_set_layout),
                                                .pushConstantRangeCount = 0};
    m_layout = vk::raii::PipelineLayout{device, layout_info};
  }

  auto       &operator()()       &{ return m_layout; }
  const auto &operator()() const & { return m_layout; }
};

} // namespace ezvk