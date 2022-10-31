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
#include "vertex.hpp"

#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace triangles {

const std::vector<throttle::graphics::vertex> vertices{
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}};

constexpr int     MAX_FRAMES_IN_FLIGHT = 2;
class application final {
public:
  application()
      : m_instance_data{std::make_unique<throttle::graphics::instance_data>()},
        m_phys_device{throttle::graphics::pick_physical_device(m_instance_data->instance())},
        m_surface_data{std::make_unique<throttle::graphics::surface_data>(
            m_instance_data->instance(), "Triangles intersection", vk::Extent2D{800, 600})},
        m_logical_device{throttle::graphics::create_device(m_phys_device, m_surface_data->surface())},
        m_queues{m_phys_device, m_logical_device, m_surface_data->surface()},
        m_swapchain_data{std::make_unique<throttle::graphics::swapchain_data>(
            m_phys_device, m_logical_device, m_surface_data->surface(), m_surface_data->extent())},
        m_pipeline_data{m_logical_device, "shaders/vertex.spv", "shaders/fragment.spv", m_surface_data->extent()},
        m_framebuffers{m_logical_device, m_swapchain_data->image_views(), m_swapchain_data->extent(),
                       m_pipeline_data.m_render_pass},
        m_command_pool{throttle::graphics::create_command_pool(m_logical_device, m_queues)},
        m_vertex_buffer{m_phys_device, m_logical_device, throttle::graphics::sizeof_vector(vertices),
                        vk::BufferUsageFlagBits::eVertexBuffer, vertices} {
    create_command_buffers();
    create_sync_objs();
  }

  void run() {
    while (!glfwWindowShouldClose(m_surface_data->window())) {
      glfwPollEvents();
      render_frame();
    }
    m_logical_device.waitIdle();
  }

private:
  std::unique_ptr<throttle::graphics::i_instance_data> m_instance_data{nullptr};
  vk::raii::PhysicalDevice         m_phys_device{nullptr};
  std::unique_ptr<throttle::graphics::i_surface_data>  m_surface_data{nullptr};
  vk::raii::Device                                     m_logical_device{nullptr};
  throttle::graphics::queues                           m_queues;
  std::unique_ptr<throttle::graphics::i_swapchain_data>               m_swapchain_data{nullptr};
  throttle::graphics::pipeline_data                     m_pipeline_data{nullptr};
  throttle::graphics::framebuffers                                    m_framebuffers;
  vk::raii::CommandPool                                 m_command_pool{nullptr};
  throttle::graphics::buffer                            m_vertex_buffer{nullptr};
  throttle::graphics::buffer                            m_index_buffer{nullptr};
  vk::raii::CommandBuffers                              m_command_buffers{nullptr};
  std::vector<vk::raii::Semaphore>                      m_image_availible_semaphores;
  std::vector<vk::raii::Semaphore>                      m_render_finished_semaphores;
  std::vector<vk::raii::Fence>                                        m_in_flight_fences;
  std::size_t                                           m_curr_frame = 0;
  bool                                                  m_framebuffer_resized = false;

private:
  void create_command_buffers() {
    uint32_t                      n_frame_buffers = m_framebuffers.size();
    vk::CommandBufferAllocateInfo alloc_info{};
    alloc_info.commandPool = *m_command_pool;
    alloc_info.commandBufferCount = n_frame_buffers;
    alloc_info.level = vk::CommandBufferLevel::ePrimary;

    try {
      m_command_buffers = vk::raii::CommandBuffers(m_logical_device, alloc_info);
    } catch (vk::SystemError &) {
      throw std::runtime_error("failed to allocate command buffers");
    }
    for (uint32_t i = 0; i < m_command_buffers.size(); ++i) {
      vk::CommandBufferBeginInfo begin_info{};
      begin_info.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
      try {
        m_command_buffers[i].begin(begin_info);
      } catch (vk::SystemError &) {
        throw std::runtime_error("failed to begin recording command buffer");
      }
      vk::RenderPassBeginInfo render_pass_info{};
      render_pass_info.renderPass = *m_pipeline_data.m_render_pass;
      render_pass_info.framebuffer = *m_framebuffers[i];
      render_pass_info.renderArea.offset = vk::Offset2D{0, 0};
      render_pass_info.renderArea.extent = m_swapchain_data->extent();
      vk::ClearValue clear_color = {std::array<float, 4>{0.2f, 0.2f, 0.2f, 0.2f}};
      render_pass_info.clearValueCount = 1;
      render_pass_info.pClearValues = &clear_color;
      m_command_buffers[i].beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
      m_command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline_data.m_pipeline);
      vk::Buffer     vertex_buffers[] = {*m_vertex_buffer.m_buffer};
      vk::DeviceSize offsets[] = {0};
      m_command_buffers[i].bindVertexBuffers(0, vertex_buffers, offsets);
      m_command_buffers[i].draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
      m_command_buffers[i].endRenderPass();
      try {
        m_command_buffers[i].end();
      } catch (vk::SystemError &) {
        throw std::runtime_error("failed to record command buffer");
      }
    }
  }

  void create_sync_objs() {
    m_image_availible_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    m_render_finished_semaphores.reserve(MAX_FRAMES_IN_FLIGHT);
    m_in_flight_fences.reserve(MAX_FRAMES_IN_FLIGHT);
    try {
      for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        m_image_availible_semaphores.push_back(m_logical_device.createSemaphore({}));
        m_render_finished_semaphores.push_back(m_logical_device.createSemaphore({}));
        m_in_flight_fences.push_back(m_logical_device.createFence({vk::FenceCreateFlagBits::eSignaled}));
      }
    } catch (vk::SystemError &) {
      throw std::runtime_error("failed to create synchronisation objects for a frame");
    }
  }

  void recreate_swap_chain() {
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(m_surface_data->window(), &width, &height);
      glfwWaitEvents();
    }
    m_swapchain_data = std::make_unique<throttle::graphics::swapchain_data>(
        m_phys_device, m_logical_device, m_surface_data->surface(), m_surface_data->extent());
    m_pipeline_data = throttle::graphics::pipeline_data{m_logical_device, "shaders/vertex.spv", "shaders/fragment.spv",
                                                        m_swapchain_data->extent()};
    m_framebuffers = throttle::graphics::framebuffers{m_logical_device, m_swapchain_data->image_views(),
                                                      m_swapchain_data->extent(), m_pipeline_data.m_render_pass};
    create_command_buffers();
    create_sync_objs();
  }

  void render_frame() {
    m_logical_device.waitForFences({*m_in_flight_fences[m_curr_frame]}, VK_TRUE, UINT64_MAX);
    uint32_t   image_index{};
    try {
      vk::AcquireNextImageInfoKHR acquire_info{};
      acquire_info.swapchain = *m_swapchain_data->swapchain();
      acquire_info.timeout = UINT64_MAX;
      acquire_info.semaphore = *m_image_availible_semaphores[m_curr_frame];
      acquire_info.fence = nullptr;
      acquire_info.deviceMask = 1;
      auto result = m_logical_device.acquireNextImage2KHR(acquire_info);
      image_index = result.second;
    } catch (vk::OutOfDateKHRError &) {
      recreate_swap_chain();
      return;
    } catch (vk::SystemError &) {
      throw std::runtime_error("failed to acquire swap chain image");
    }

    vk::SubmitInfo         submit_info{};
    vk::Semaphore          wait_semaphores[] = {*m_image_availible_semaphores[m_curr_frame]};
    vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &(*m_command_buffers[image_index]);

    vk::Semaphore signal_semaphores[] = {*m_render_finished_semaphores[m_curr_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    m_logical_device.resetFences({*m_in_flight_fences[m_curr_frame]});

    try {
      m_queues.graphics.submit(submit_info, *m_in_flight_fences[m_curr_frame]);
    } catch (vk::SystemError &) {
      throw std::runtime_error("failed to submit sraw command buffer");
    }

    vk::PresentInfoKHR present_info{};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    vk::SwapchainKHR swap_chains[] = {*m_swapchain_data->swapchain()};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;

    vk::Result result_present;
    try {
      result_present = m_queues.present.presentKHR(present_info);
    } catch (vk::OutOfDateKHRError &) {
      result_present = vk::Result::eErrorOutOfDateKHR;
    } catch (vk::SystemError &) {
      throw std::runtime_error("failed to present swap chain image");
    }

    if (result_present == vk::Result::eSuboptimalKHR || m_framebuffer_resized) {
      m_framebuffer_resized = false;
      recreate_swap_chain();
      return;
    }
  }
};
} // namespace triangles