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

#include "depth_buffer.hpp"
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

static inline vk::AttachmentDescription create_depth_attachment(vk::Format depth_format) {
  return {.flags = vk::AttachmentDescriptionFlags{},
          .format = depth_format,
          .samples = vk::SampleCountFlagBits::e1,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eDontCare,
          .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
          .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
          .initialLayout = vk::ImageLayout::eUndefined,
          .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal};
}

class render_pass final {
  vk::raii::RenderPass m_render_pass = nullptr;

public:
  render_pass() = default;

  render_pass(const vk::raii::Device &device, const vk::SubpassDescription &subpass,
              std::span<vk::AttachmentDescription> attachments = {}, std::span<const vk::SubpassDependency> deps = {}) {

    vk::RenderPassCreateInfo renderpass_info = {.attachmentCount = static_cast<uint32_t>(attachments.size()),
                                                .pAttachments = attachments.data(),
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