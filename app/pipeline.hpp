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

#include "unified_includes/vulkan_hpp_include.hpp"

#include <numeric>
#include <tuple>
#include <vector>

#include "ezvk/wrappers/descriptor_set.hpp"
#include "ezvk/wrappers/memory.hpp"
#include "ezvk/wrappers/shaders.hpp"

#include "vertex.hpp"

namespace triangles {

template <typename t_vertex_type> class pipeline final {
  vk::raii::Pipeline m_pipeline = nullptr;

public:
  pipeline() = default;

  pipeline(const vk::raii::Device &p_device, const std::string &p_vertex_file_path,
      const std::string &p_fragment_file_path, const vk::raii::PipelineLayout &p_pipeline_layout,
      const vk::raii::RenderPass &p_render_pass,
      const vk::PipelineRasterizationStateCreateInfo &rasterization_state_info,
      const vk::PrimitiveTopology primitive_topology) {
    auto binding_description = t_vertex_type::get_binding_description();
    auto attribute_description = t_vertex_type::get_attribute_description();

    auto vertex_input_info = vertex_input_state_create_info(binding_description, attribute_description);
    vk::PipelineColorBlendAttachmentState color_attachments = {.blendEnable = VK_FALSE,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

    const auto color_blend_info = color_blend_state_create_info(color_attachments);

    vk::PipelineInputAssemblyStateCreateInfo input_asm_info = {
        .flags = vk::PipelineInputAssemblyStateCreateFlags(), .topology = primitive_topology};

    std::array<vk::DynamicState, 2> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    vk::PipelineViewportStateCreateInfo viewport_info = {
        .viewportCount = 1, .pViewports = nullptr, .scissorCount = 1, .pScissors = nullptr};

    vk::PipelineDynamicStateCreateInfo dynamic_state_info = {
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()), .pDynamicStates = dynamic_states.data()};

    vk::PipelineMultisampleStateCreateInfo multisampling = {
        .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = VK_FALSE};

    std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;

    // vertex shader
    auto vertex_shader = ezvk::create_module(p_vertex_file_path, p_device);
    vk::PipelineShaderStageCreateInfo vertex_shader_info = {
        .stage = vk::ShaderStageFlagBits::eVertex, .module = *vertex_shader, .pName = "main"};
    shader_stages.push_back(vertex_shader_info);

    // fragment shader
    auto fragment_shader = ezvk::create_module(p_fragment_file_path, p_device);
    vk::PipelineShaderStageCreateInfo fragment_shader_info = {
        .stage = vk::ShaderStageFlagBits::eFragment, .module = *fragment_shader, .pName = "main"};
    shader_stages.push_back(fragment_shader_info);

    vk::PipelineDepthStencilStateCreateInfo depth_stencil = {.depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f};

    vk::GraphicsPipelineCreateInfo pipeline_info = {.stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_asm_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterization_state_info,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = &dynamic_state_info,
        .layout = *p_pipeline_layout,
        .renderPass = *p_render_pass,
        .subpass = 0,
        .basePipelineHandle = nullptr};

    m_pipeline = p_device.createGraphicsPipeline(nullptr, pipeline_info);
  }

  static vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info(
      vk::VertexInputBindingDescription &binding_description, const auto &attribute_description) {
    return vk::PipelineVertexInputStateCreateInfo{.flags = vk::PipelineVertexInputStateCreateFlags(),
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size()),
        .pVertexAttributeDescriptions = attribute_description.data()};
  }

  static vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info(
      vk::PipelineColorBlendAttachmentState &color_attachments) {
    return vk::PipelineColorBlendStateCreateInfo{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_attachments,
        .blendConstants = {},
    };
  }

  auto &operator()() & { return m_pipeline; }
  const auto &operator()() const & { return m_pipeline; }
};

} // namespace triangles