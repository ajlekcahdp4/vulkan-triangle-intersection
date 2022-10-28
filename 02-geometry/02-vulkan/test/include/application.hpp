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

#include "device.hpp"
#include "instance.hpp"
#include "memory.hpp"
#include "pipeline.hpp"
#include "queue_families.hpp"
#include "render_pass.hpp"
#include "surface.hpp"
#include "swapchain.hpp"

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace triangles {

const std::vector<throttle::graphics::vertex> Vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                                          {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                                          {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                                          {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

const std::vector<unsigned short> Indices = {0, 1, 2, 2, 3, 0};
class application final {
public:
  application()
      : m_instance_data{std::make_unique<throttle::graphics::instance_data>()},
        m_phys_device{throttle::graphics::pick_physical_device(m_instance_data->instance())},
        m_surface_data{std::make_unique<throttle::graphics::surface_data>(*m_instance_data, "Triangles intersection",
                                                                          vk::Extent2D{800, 600})},
        m_logical_device{throttle::graphics::create_device(m_phys_device, *m_surface_data)}, m_queues{m_phys_device,
                                                                                                      m_logical_device,
                                                                                                      *m_surface_data},
        m_swapchain_data{std::make_unique<throttle::graphics::swapchain_data>(
            m_phys_device, m_logical_device, *m_surface_data, m_surface_data->extent())},
        m_pipeline_data{m_logical_device, "shaders/vertex.spv", "shaders/fragment.spv", m_surface_data->extent()},
        m_framebuffers{throttle::graphics::allocate_frame_buffers(m_logical_device, *m_swapchain_data, *m_surface_data,
                                                                                  m_pipeline_data)},
        m_command_pool{throttle::graphics::create_command_pool(m_logical_device, m_queues)},
        m_vertex_buffer{m_phys_device, m_logical_device, throttle::graphics::sizeof_vector(Vertices),
                        vk::BufferUsageFlagBits::eVertexBuffer},
        m_index_buffer{m_phys_device, m_logical_device, throttle::graphics::sizeof_vector(Indices),
                       vk::BufferUsageFlagBits::eIndexBuffer} {}

  void run() {
    while (!glfwWindowShouldClose(m_surface_data->window()))
      glfwPollEvents();
  }

private:
  std::unique_ptr<throttle::graphics::i_instance_data> m_instance_data{nullptr};
  vk::raii::PhysicalDevice         m_phys_device{nullptr};
  std::unique_ptr<throttle::graphics::i_surface_data>  m_surface_data{nullptr};
  vk::raii::Device                                     m_logical_device{nullptr};
  throttle::graphics::queues                           m_queues;
  std::unique_ptr<throttle::graphics::i_swapchain_data> m_swapchain_data{nullptr};
  throttle::graphics::pipeline_data                     m_pipeline_data{nullptr};
  std::vector<vk::raii::Framebuffer>                    m_framebuffers;
  vk::raii::CommandPool                                 m_command_pool{nullptr};
  throttle::graphics::buffer                            m_vertex_buffer{nullptr};
  throttle::graphics::buffer                            m_index_buffer{nullptr};
  vk::raii::CommandBuffers                              m_command_buffers{nullptr};

private:
  void create_command_buffers(const std::vector<unsigned short> &p_indices) {
                    uint32_t                      n_frame_buffers = m_framebuffers.size();
                    vk::CommandBufferAllocateInfo alloc_info{};
                    alloc_info.commandPool = *m_command_pool;
                    alloc_info.commandBufferCount = n_frame_buffers;
                    vk::raii::CommandBuffers command_buffers{m_logical_device, alloc_info};
                    for (uint32_t i = 0; i < command_buffers.size(); i++) {
                      vk::CommandBufferBeginInfo begin_info{};
                      command_buffers[i].begin(begin_info);
                      vk::RenderPassBeginInfo render_pass_info{};
                      render_pass_info.renderPass = *m_pipeline_data.m_render_pass;
                      render_pass_info.framebuffer = *m_framebuffers[i];
                      render_pass_info.renderArea.offset = vk::Offset2D{0, 0};
                      render_pass_info.renderArea.extent = m_swapchain_data->extent();
                      vk::ClearValue clear_color = vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.2f, 0.3f, 0.3f, 1.0f}}};
                      render_pass_info.clearValueCount = 1;
                      render_pass_info.pClearValues = &clear_color;
                      command_buffers[i].beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
                      command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline_data.m_pipeline);
                      vk::Buffer     vertex_buffers[] = {*m_vertex_buffer.m_buffer};
                      vk::DeviceSize offsets[] = {0};
                      command_buffers[i].bindVertexBuffers(0, {vertex_buffers}, {offsets});
                      command_buffers[i].bindIndexBuffer(*m_index_buffer.m_buffer, 0, vk::IndexType::eUint16);
                      command_buffers[i].drawIndexed(p_indices.size(), 1, 0, 0, 0);
                      command_buffers[i].endRenderPass();
                      command_buffers[i].end();
    }
  }
};
} // namespace triangles