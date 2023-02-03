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

#include <numeric>
#include <tuple>
#include <vector>

#include "ezvk/descriptor_set.hpp"
#include "ezvk/memory.hpp"
#include "shaders.hpp"

namespace throttle::graphics {

template <class vertex_t> struct pipeline_data {
public:
  vk::raii::RenderPass     m_render_pass{nullptr};
  vk::raii::PipelineLayout m_layout{nullptr};
  vk::raii::Pipeline       m_pipeline{nullptr};

  pipeline_data(std::nullptr_t) {}

  pipeline_data(const vk::raii::Device &p_device, const std::string &p_vertex_file_path,
                const std::string &p_fragment_file_path, const ezvk::descriptor_set &p_descriptor_set_data)
      : m_render_pass{create_render_pass(p_device)}, m_layout{create_pipeline_layout(p_device,
                                                                                     p_descriptor_set_data.m_layout)},
        m_pipeline{create_pipeline(p_device, p_vertex_file_path, p_fragment_file_path)} {}

private:
  vk::raii::Pipeline create_pipeline(const vk::raii::Device &p_device, const std::string &p_vertex_file_path,
                                     const std::string &p_fragment_file_path);

  static vk::raii::PipelineLayout create_pipeline_layout(const vk::raii::Device              &device,
                                                         const vk::raii::DescriptorSetLayout &p_descriptor_set_layout);

  static vk::raii::RenderPass create_render_pass(const vk::raii::Device &p_device);

  static vk::PipelineVertexInputStateCreateInfo
  vertex_input_state_create_info(vk::VertexInputBindingDescription                  &binding_description,
                                 std::array<vk::VertexInputAttributeDescription, 2> &attribute_description);

  static vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info();

  static vk::PipelineColorBlendStateCreateInfo
  color_blend_state_create_info(vk::PipelineColorBlendAttachmentState &color_attachments);

  static vk::PipelineColorBlendAttachmentState color_blend_attachments();
};

template <class vertex_t>
vk::raii::Pipeline pipeline_data<vertex_t>::create_pipeline(const vk::raii::Device &p_device,
                                                            const std::string      &p_vertex_file_path,
                                                            const std::string      &p_fragment_file_path) {
  auto binding_description = vertex_t::get_binding_description();
  auto attribute_description = vertex_t::get_attribute_description();
  auto vertex_input_info = vertex_input_state_create_info(binding_description, attribute_description);
  auto rasterization_info = rasterization_state_create_info();
  auto color_attachments = color_blend_attachments();
  auto color_blend_info = color_blend_state_create_info(color_attachments);
  vk::PipelineInputAssemblyStateCreateInfo input_asm_info{.flags = vk::PipelineInputAssemblyStateCreateFlags(),
                                                          .topology = vk::PrimitiveTopology::eTriangleList};

  auto             dynamic_state_viewport = vk::DynamicState::eViewport;
  auto             dynamic_state_scissor = vk::DynamicState::eScissor;
  vk::DynamicState dynamic_states[] = {dynamic_state_viewport, dynamic_state_scissor};

  vk::PipelineViewportStateCreateInfo viewport_info = {
      .viewportCount = 1, .pViewports = nullptr, .scissorCount = 1, .pScissors = nullptr};

  vk::PipelineDynamicStateCreateInfo dynamic_state_info = {.dynamicStateCount = 2, .pDynamicStates = dynamic_states};

  vk::PipelineMultisampleStateCreateInfo         multisampling = {.rasterizationSamples = vk::SampleCountFlagBits::e1,
                                                                  .sampleShadingEnable = VK_FALSE};
  std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;

  // vertex shader
  auto                              vertex_shader = create_module(p_vertex_file_path, p_device);
  vk::PipelineShaderStageCreateInfo vertex_shader_info = {
      .stage = vk::ShaderStageFlagBits::eVertex, .module = *vertex_shader, .pName = "main"};
  shader_stages.push_back(vertex_shader_info);

  // fragment shader
  auto                              fragment_shader = create_module(p_fragment_file_path, p_device);
  vk::PipelineShaderStageCreateInfo fragment_shader_info = {
      .stage = vk::ShaderStageFlagBits::eFragment, .module = *fragment_shader, .pName = "main"};
  shader_stages.push_back(fragment_shader_info);

  vk::GraphicsPipelineCreateInfo pipeline_info = {.stageCount = static_cast<uint32_t>(shader_stages.size()),
                                                  .pStages = shader_stages.data(),
                                                  .pVertexInputState = &vertex_input_info,
                                                  .pInputAssemblyState = &input_asm_info,
                                                  .pViewportState = &viewport_info,
                                                  .pRasterizationState = &rasterization_info,
                                                  .pMultisampleState = &multisampling,
                                                  .pColorBlendState = &color_blend_info,
                                                  .pDynamicState = &dynamic_state_info,
                                                  .layout = *m_layout,
                                                  .renderPass = *m_render_pass,
                                                  .subpass = 0,
                                                  .basePipelineHandle = nullptr};

  return p_device.createGraphicsPipeline(nullptr, pipeline_info);
}

template <class vertex_t>
vk::raii::PipelineLayout
pipeline_data<vertex_t>::create_pipeline_layout(const vk::raii::Device              &p_device,
                                                const vk::raii::DescriptorSetLayout &p_descriptor_set_layout) {
  vk::PipelineLayoutCreateInfo layout_info{};
  layout_info.flags = vk::PipelineLayoutCreateFlags();
  layout_info.setLayoutCount = 1;
  vk::DescriptorSetLayout set_layouts[] = {*p_descriptor_set_layout};
  layout_info.pSetLayouts = set_layouts;
  layout_info.pushConstantRangeCount = 0;
  return vk::raii::PipelineLayout{p_device, layout_info};
}

template <class vertex_t>
vk::raii::RenderPass pipeline_data<vertex_t>::create_render_pass(const vk::raii::Device &p_device) {
  vk::AttachmentDescription color_attachment{.flags = vk::AttachmentDescriptionFlags(),
                                             .format = vk::Format::eB8G8R8A8Unorm,
                                             .samples = vk::SampleCountFlagBits::e1,
                                             .loadOp = vk::AttachmentLoadOp::eClear,
                                             .storeOp = vk::AttachmentStoreOp::eStore,
                                             .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                             .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                                             .initialLayout = vk::ImageLayout::eUndefined,
                                             .finalLayout = vk::ImageLayout::ePresentSrcKHR};

  vk::AttachmentReference color_attachment_ref{.attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpass = {.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                    .colorAttachmentCount = 1,
                                    .pColorAttachments = &color_attachment_ref};

  vk::RenderPassCreateInfo renderpass_info{
      .attachmentCount = 1, .pAttachments = &color_attachment, .subpassCount = 1, .pSubpasses = &subpass};

  return p_device.createRenderPass(renderpass_info);
}

template <class vertex_t>
vk::PipelineVertexInputStateCreateInfo pipeline_data<vertex_t>::vertex_input_state_create_info(
    vk::VertexInputBindingDescription                  &binding_description,
    std::array<vk::VertexInputAttributeDescription, 2> &attribute_description) {
  return vk::PipelineVertexInputStateCreateInfo{.flags = vk::PipelineVertexInputStateCreateFlags(),
                                                .vertexBindingDescriptionCount = 1,
                                                .pVertexBindingDescriptions = &binding_description,
                                                .vertexAttributeDescriptionCount =
                                                    static_cast<uint32_t>(attribute_description.size()),
                                                .pVertexAttributeDescriptions = attribute_description.data()

  };
}

template <class vertex_t>
vk::PipelineRasterizationStateCreateInfo pipeline_data<vertex_t>::rasterization_state_create_info() {
  return vk::PipelineRasterizationStateCreateInfo{
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eNone,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };
}

template <class vertex_t> vk::PipelineColorBlendAttachmentState pipeline_data<vertex_t>::color_blend_attachments() {
  return vk::PipelineColorBlendAttachmentState{
      .blendEnable = VK_FALSE,
      .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
  };
}

template <class vertex_t>
vk::PipelineColorBlendStateCreateInfo
pipeline_data<vertex_t>::color_blend_state_create_info(vk::PipelineColorBlendAttachmentState &color_attachments) {
  return vk::PipelineColorBlendStateCreateInfo{
      .logicOpEnable = VK_FALSE,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &color_attachments,
      .blendConstants = {},
  };
}

} // namespace throttle::graphics