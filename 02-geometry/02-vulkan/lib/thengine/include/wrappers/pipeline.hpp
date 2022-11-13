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

#include "primitives/vertex.hpp"

#include "shaders.hpp"

#include "vulkan_include.hpp"

#include <numeric>

namespace throttle {
namespace graphics {

struct descriptor_set_data {
public:
  vk::raii::DescriptorSetLayout m_layout = nullptr;
  vk::raii::DescriptorPool      m_pool = nullptr;
  vk::raii::DescriptorSet       m_descriptor_set = nullptr;

  descriptor_set_data(std::nullptr_t) {}
  descriptor_set_data(const vk::raii::Device &p_device)
      : m_layout{create_decriptor_set_layout(
            p_device, {{vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                       {vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}})},
        m_pool{std::move(create_descriptor_pool(
            p_device, {{vk::DescriptorType::eUniformBuffer, 1}, {vk::DescriptorType::eCombinedImageSampler, 1}}))},
        m_descriptor_set{std::move((vk::raii::DescriptorSets{p_device, {*m_pool, *m_layout}}).front())} {}

private:
  static vk::raii::DescriptorSetLayout create_decriptor_set_layout(
      const vk::raii::Device                                                            &p_device,
      const std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> &p_binding_data) {
    std::vector<vk::DescriptorSetLayoutBinding> bindings{static_cast<uint32_t>(p_binding_data.size())};

    for (uint32_t i = 0; i < p_binding_data.size(); ++i) {
      bindings.push_back(vk::DescriptorSetLayoutBinding{
          i, std::get<0>(p_binding_data[i]), std::get<1>(p_binding_data[i]), std::get<2>(p_binding_data[i])});
    }

    vk::DescriptorSetLayoutCreateInfo descriptor_set_info = {.bindingCount = static_cast<uint32_t>(bindings.size()),
                                                             .pBindings = bindings.data()};

    return vk::raii::DescriptorSetLayout{p_device, descriptor_set_info};
  }

  static vk::raii::DescriptorPool create_descriptor_pool(const vk::raii::Device                    &p_device,
                                                         const std::vector<vk::DescriptorPoolSize> &p_pool_sizes) {
    uint32_t max_sets =
        std::accumulate(p_pool_sizes.begin(), p_pool_sizes.end(), 0,
                        [](uint32_t sum, vk::DescriptorPoolSize const &dps) { return sum + dps.descriptorCount; });

    vk::DescriptorPoolCreateInfo descriptor_info = {.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                    .maxSets = max_sets,
                                                    .poolSizeCount = static_cast<uint32_t>(p_pool_sizes.size()),
                                                    .pPoolSizes = p_pool_sizes.data()};

    return vk::raii::DescriptorPool(p_device, descriptor_info);
  }
};

struct pipeline_data {
public:
  vk::raii::RenderPass     m_render_pass{nullptr};
  vk::raii::PipelineLayout m_layout{nullptr};
  vk::raii::Pipeline       m_pipeline{nullptr};

  pipeline_data(std::nullptr_t) {}

  pipeline_data(const vk::raii::Device &p_device, const std::string &p_vertex_file_path,
                const std::string &p_fragment_file_path, const vk::Extent2D &p_extent,
                const descriptor_set_data &p_descriptor_set_data)
      : m_render_pass{create_render_pass(p_device)}, m_layout{create_pipeline_layout(p_device,
                                                                                     p_descriptor_set_data.m_layout)},
        m_pipeline{create_pipeline(p_device, p_vertex_file_path, p_fragment_file_path, p_extent)} {}

private:
  vk::raii::Pipeline create_pipeline(const vk::raii::Device &p_device, const std::string &p_vertex_file_path,
                                     const std::string &p_fragment_file_path, const vk::Extent2D &p_extent) {
    // vertex input
    vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.flags = vk::PipelineVertexInputStateCreateFlags();
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    auto binding_description = vertex::get_binding_description();
    auto attribute_description = vertex::get_attribute_description();
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = attribute_description.size();
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.pVertexAttributeDescriptions = attribute_description.data();

    // input assembly
    vk::PipelineInputAssemblyStateCreateInfo input_asm_info{};
    input_asm_info.flags = vk::PipelineInputAssemblyStateCreateFlags();
    input_asm_info.topology = vk::PrimitiveTopology::eTriangleList;

    // Viewport and scissor
    vk::Viewport viewport = {0.0f, 0.0f, static_cast<float>(p_extent.width), static_cast<float>(p_extent.height),
                             0.0f, 1.0f};
    vk::Rect2D   scissor = {vk::Offset2D{0, 0}, p_extent};

    vk::PipelineViewportStateCreateInfo viewport_info = {
        .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor};

    // Rasterisation
    vk::PipelineRasterizationStateCreateInfo rasterization_info = {
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    // Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling = {.rasterizationSamples = vk::SampleCountFlagBits::e1,
                                                            .sampleShadingEnable = VK_FALSE};

    // color blend
    vk::PipelineColorBlendAttachmentState color_blend_attachments = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
    };

    vk::PipelineColorBlendStateCreateInfo color_blending = {
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachments,
        .blendConstants = {},
    };

    // shader stages
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
                                                    .pColorBlendState = &color_blending,
                                                    .pMultisampleState = &multisampling,
                                                    .subpass = 0,
                                                    .basePipelineHandle = nullptr,
                                                    .renderPass = *m_render_pass,
                                                    .layout = *m_layout};

    return p_device.createGraphicsPipeline(nullptr, pipeline_info);
  }

  static vk::raii::PipelineLayout create_pipeline_layout(const vk::raii::Device              &device,
                                                         const vk::raii::DescriptorSetLayout &p_descriptor_set_layout) {
    vk::PipelineLayoutCreateInfo layout_info{};
    layout_info.flags = vk::PipelineLayoutCreateFlags();
    layout_info.setLayoutCount = 1;
    vk::DescriptorSetLayout set_layouts[] = {*p_descriptor_set_layout};
    layout_info.pSetLayouts = set_layouts;
    layout_info.pushConstantRangeCount = 0;
    return device.createPipelineLayout(layout_info);
  }

  static vk::raii::RenderPass create_render_pass(const vk::raii::Device &p_device) {
    vk::AttachmentDescription color_attachment = {.flags = vk::AttachmentDescriptionFlags(),
                                                  .format = vk::Format::eB8G8R8A8Unorm,
                                                  .samples = vk::SampleCountFlagBits::e1,
                                                  .loadOp = vk::AttachmentLoadOp::eClear,
                                                  .storeOp = vk::AttachmentStoreOp::eStore,
                                                  .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                                                  .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                                                  .initialLayout = vk::ImageLayout::eUndefined,
                                                  .finalLayout = vk::ImageLayout::ePresentSrcKHR};

    vk::AttachmentReference color_attachment_ref = {.attachment = 0,
                                                    .layout = vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass = {.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                      .colorAttachmentCount = 1,
                                      .pColorAttachments = &color_attachment_ref};

    vk::RenderPassCreateInfo renderpass_info = {
        .attachmentCount = 1, .pAttachments = &color_attachment, .subpassCount = 1, .pSubpasses = &subpass};

    return p_device.createRenderPass(renderpass_info);
  }
}; // namespace graphics
} // namespace graphics
} // namespace throttle