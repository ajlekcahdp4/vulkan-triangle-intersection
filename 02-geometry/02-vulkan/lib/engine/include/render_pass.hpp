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

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "instance.hpp"
#include "window.hpp"

namespace throttle {
namespace graphics {
inline vk::raii::RenderPass create_render_pass(const vk::raii::Device &p_device) {
  vk::AttachmentDescription color_attachment = {};
  color_attachment.flags = vk::AttachmentDescriptionFlags();
  color_attachment.format = vk::Format::eB8G8R8A8Unorm;
  color_attachment.samples = vk::SampleCountFlagBits::e1;
  color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
  color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
  color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  color_attachment.initialLayout = vk::ImageLayout::eUndefined;
  color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

  vk::AttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass = {};
  subpass.flags = vk::SubpassDescriptionFlags();
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  vk::RenderPassCreateInfo renderpass_info = {};
  renderpass_info.flags = vk::RenderPassCreateFlags();
  renderpass_info.attachmentCount = 1;
  renderpass_info.pAttachments = &color_attachment;
  renderpass_info.subpassCount = 1;
  renderpass_info.pSubpasses = &subpass;

  return p_device.createRenderPass(renderpass_info);
}
} // namespace graphics
} // namespace throttle