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

#include <engine.hpp>

#include "misc.hpp"

#include "ezvk/debug.hpp"
#include "ezvk/instance.hpp"
#include "ezvk/queues.hpp"
#include "ezvk/shaders.hpp"
#include "ezvk/window.hpp"

#include "glfw_include.hpp"
#include "vulkan_include.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>

#if defined(VK_VALIDATION_LAYER) || !defined(NDEBUG)
#define USE_DEBUG_EXTENSION
#endif

namespace triangles {

const std::vector<throttle::graphics::vertex> vertices{{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                                       {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                                       {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                                       {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

struct vk_requirements {
  std::vector<std::string> extensions, layers;

  vk_requirements() {
    extensions = required_vk_extensions();
    layers = required_vk_layers(true);
  }
};

constexpr int     MAX_FRAMES_IN_FLIGHT = 2;
class application final {
#ifdef USE_DEBUG_EXTENSION
  ezvk::debugged_instance m_instance;
#else
  ezvk::instance m_instance;
#endif

  vk::raii::PhysicalDevice m_phys_device = nullptr;
  ezvk::unique_glfw_window m_window = nullptr;
  ezvk::surface            m_surface;
  vk::raii::Device         m_logical_device = nullptr;

  throttle::graphics::queues                                    m_queues = nullptr;
  throttle::graphics::swapchain_wrapper                         m_swapchain_data = nullptr;
  throttle::graphics::buffers                                   m_uniform_buffers;
  throttle::graphics::descriptor_set_data                       m_descriptor_set_data = nullptr;
  throttle::graphics::pipeline_data<throttle::graphics::vertex> m_pipeline_data = nullptr;
  throttle::graphics::framebuffers                              m_framebuffers;
  vk::raii::CommandPool                                         m_command_pool = nullptr;
  throttle::graphics::buffer                                    m_vertex_buffer = nullptr;
  throttle::graphics::buffer                                    m_index_buffer = nullptr;
  vk::raii::CommandBuffers                                      m_command_buffers = nullptr;
  std::vector<vk::raii::Semaphore>                              m_image_availible_semaphores;
  std::vector<vk::raii::Semaphore>                              m_render_finished_semaphores;
  std::vector<vk::raii::Fence>                                  m_in_flight_fences;
  std::size_t                                                   m_curr_frame = 0;
  bool                                                          m_framebuffer_resized = false;

public:
#ifdef USE_DEBUG_EXTENSION
  application(ezvk::debugged_instance &&p_instance){
#else
  application(ezvk::instance &&p_instance) {
#endif
      m_instance = std::move(p_instance);

  m_phys_device = throttle::graphics::pick_physical_device(m_instance());
  m_window = {"Triangles intersection", vk::Extent2D{800, 600}, true};

  m_surface = {m_instance(), m_window};

  m_logical_device = {throttle::graphics::create_device(m_phys_device, m_surface())};
  m_queues = {m_phys_device, m_logical_device, m_surface()};

  m_swapchain_data = {m_phys_device, m_logical_device, m_surface(), m_window.extent()};
  m_uniform_buffers = {MAX_FRAMES_IN_FLIGHT, sizeof(throttle::graphics::uniform_buffer_object), m_phys_device,
                       m_logical_device, vk::BufferUsageFlagBits::eUniformBuffer};
  m_descriptor_set_data = {m_logical_device, m_uniform_buffers};

  m_pipeline_data = {m_logical_device, "shaders/vertex.spv", "shaders/fragment.spv", m_window.extent(),
                     m_descriptor_set_data};
  m_framebuffers = {m_logical_device, m_swapchain_data.image_views(), m_swapchain_data.extent(),
                    m_pipeline_data.m_render_pass};

  m_command_pool = {throttle::graphics::create_command_pool(m_logical_device, m_queues)};
  m_vertex_buffer = {m_phys_device, m_logical_device, vk::BufferUsageFlagBits::eVertexBuffer, vertices};
  m_index_buffer = {m_phys_device, m_logical_device, vk::BufferUsageFlagBits::eIndexBuffer, indices};

  create_command_buffers();
  create_sync_objs();
}

  void run() {
  while (!glfwWindowShouldClose(m_window())) {
    glfwPollEvents();
    render_frame();
  }
  m_logical_device.waitIdle();
}

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
    render_pass_info.renderArea.extent = m_swapchain_data.extent();
    vk::ClearValue clear_color = {std::array<float, 4>{0.2f, 0.3f, 0.3f, 1.0f}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;
    m_command_buffers[i].beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    m_command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline_data.m_pipeline);
    m_command_buffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipeline_data.m_layout, 0,
                                            {*m_descriptor_set_data.m_descriptor_set}, nullptr);
    vk::Buffer     vertex_buffers[] = {*m_vertex_buffer.m_buffer};
    vk::DeviceSize offsets[] = {0};
    m_command_buffers[i].bindVertexBuffers(0, vertex_buffers, offsets);
    vk::Buffer index_buffer = *m_index_buffer.m_buffer;
    m_command_buffers[i].bindIndexBuffer(index_buffer, 0, vk::IndexType::eUint16);
    m_command_buffers[i].drawIndexed(indices.size(), 1, 0, 0, 0);
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

  try {
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      m_image_availible_semaphores.push_back(m_logical_device.createSemaphore({}));
      m_render_finished_semaphores.push_back(m_logical_device.createSemaphore({}));
      m_in_flight_fences.push_back(
          m_logical_device.createFence(vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled}));
    }
  } catch (vk::SystemError &) {
    throw std::runtime_error("failed to create synchronisation objects for a frame");
  }
}

void recreate_swap_chain() {
  auto extent = m_window.extent();

  while (extent.width == 0 || extent.height == 0) {
    extent = m_window.extent();
    glfwWaitEvents();
  }

  m_logical_device.waitIdle();
  m_swapchain_data.swapchain()
      .clear(); // Destroy the old swapchain before recreating another. NOTE[Sergei]: this is a dirty fix

  m_swapchain_data = throttle::graphics::swapchain_wrapper(m_phys_device, m_logical_device, m_surface(), extent);
  m_pipeline_data = throttle::graphics::pipeline_data<throttle::graphics::vertex>{
      m_logical_device, "shaders/vertex.spv", "shaders/fragment.spv", m_swapchain_data.extent(), m_descriptor_set_data};

  m_framebuffers = throttle::graphics::framebuffers{m_logical_device, m_swapchain_data.image_views(),
                                                    m_swapchain_data.extent(), m_pipeline_data.m_render_pass};
  create_command_buffers();
}

// TEMPORARY
glm::mat4x4 create_mvpc_matrix(const vk::Extent2D &extent) {
  float fov = glm::radians(45.0f);
  if (extent.width > extent.height) fov *= static_cast<float>(extent.height) / static_cast<float>(extent.width);

  glm::mat4x4 model = glm::mat4x4(1.0f);
  glm::mat4x4 view =
      glm::lookAt(glm::vec3(-5.0f, 3.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
  glm::mat4x4 proj = glm::perspective(fov, static_cast<float>(extent.width) / extent.height, 0.1f, 100.0f);
  // clang-format off
    glm::mat4x4 clip = glm::mat4x4{ // Vulkan clip space has inverted Y and half Z
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f,  0.0f, 0.5f, 0.0f,
      0.0f,  0.0f, 0.5f, 1.0f
    };
  // clang-format on
  return clip * proj * view * model;
}

// NOTE: uniform buffers should be updated before the descriptors set's creation
void update_uniform_buffers() {
  static auto start_time = std::chrono::high_resolution_clock::now();
  auto        curr_time = std::chrono::high_resolution_clock::now();
  float       time = std::chrono::duration<float, std::chrono::seconds::period>(curr_time - start_time).count();
  throttle::graphics::uniform_buffer_object ubo{};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f),
                              m_swapchain_data.extent().width / (float)m_swapchain_data.extent().height, 0.1f, 10.0f);
  ubo.proj[1][1] *= -1; // because in OpenGl y axis is inverted
  m_uniform_buffers[m_curr_frame].copy_to_device(ubo);
}

// INSPIRATION: https://github.com/tilir/cpp-graduate/blob/master/10-3d/vk-simplest.cc
void render_frame() {
  m_logical_device.waitForFences({*m_in_flight_fences[m_curr_frame]}, VK_TRUE, UINT64_MAX);
  uint32_t image_index{};
  try {
    vk::AcquireNextImageInfoKHR acquire_info{.swapchain = *m_swapchain_data.swapchain(),
                                             .timeout = UINT64_MAX,
                                             .semaphore = *m_image_availible_semaphores[m_curr_frame],
                                             .fence = nullptr,
                                             .deviceMask = 1};

    auto result = m_logical_device.acquireNextImage2KHR(acquire_info);
    image_index = result.second;
  } catch (vk::OutOfDateKHRError &) {
    recreate_swap_chain();
    return;
  } catch (vk::SystemError &) {
    throw std::runtime_error("failed to acquire swap chain image");
  }
  auto mvpc = create_mvpc_matrix(m_swapchain_data.extent());
  m_uniform_buffers[m_curr_frame].copy_to_device(mvpc);

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

  m_logical_device.resetFences({*m_in_flight_fences[m_curr_frame]}); // segfault was there

  try {
    m_queues.graphics.submit(submit_info, *m_in_flight_fences[m_curr_frame]);
  } catch (vk::SystemError &) {
    throw std::runtime_error("failed to submit sraw command buffer");
  }

  vk::PresentInfoKHR present_info{};
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  vk::SwapchainKHR swap_chains[] = {*m_swapchain_data.swapchain()};
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

  m_curr_frame = (m_curr_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}
}; // namespace triangles
} // namespace triangles