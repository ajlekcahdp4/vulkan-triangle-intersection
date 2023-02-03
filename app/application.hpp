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

#include "equal.hpp"
#include "wrappers.hpp"

#include "camera.hpp"
#include "misc.hpp"
#include "ubo.hpp"
#include "utils.hpp"
#include "vertex.hpp"

#include "ezvk/debug.hpp"
#include "ezvk/debugged_instance.hpp"
#include "ezvk/device.hpp"
#include "ezvk/instance.hpp"
#include "ezvk/memory.hpp"
#include "ezvk/queues.hpp"
#include "ezvk/shaders.hpp"
#include "ezvk/swapchain.hpp"
#include "ezvk/window.hpp"

#include "glfw_include.hpp"
#include "glm_inlcude.hpp"
#include "vulkan_hpp_include.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <unordered_map>

#if defined(VK_VALIDATION_LAYER) || !defined(NDEBUG)
#define USE_DEBUG_EXTENSION
#endif

namespace triangles {

struct applicaton_platform {
  ezvk::generic_instance   m_instance;
  ezvk::unique_glfw_window m_window = nullptr;
  ezvk::surface            m_surface;

  vk::raii::PhysicalDevice m_p_device = nullptr;

public:
  applicaton_platform(ezvk::generic_instance instance, ezvk::unique_glfw_window window, ezvk::surface surface,
                      vk::raii::PhysicalDevice p_device)
      : m_instance{std::move(instance)}, m_window{std::move(window)}, m_surface{std::move(surface)},
        m_p_device{std::move(p_device)} {}

  auto       &instance()       &{ return m_instance(); }
  const auto &instance() const & { return m_instance(); }

  auto       &window()       &{ return m_window; }
  const auto &window() const & { return m_window; }

  auto       &surface()       &{ return m_surface(); }
  const auto &surface() const & { return m_surface(); }

  auto       &p_device()       &{ return m_p_device; }
  const auto &p_device() const & { return m_p_device; }
};

class input_handler {
public:
  enum class button_state : uint32_t { e_idle, e_held_down, e_pressed };
  using key_index = int;

private:
  struct tracked_key_info {
    button_state current_state, look_for;
  };

  std::unordered_map<key_index, tracked_key_info> m_tracked_keys;

  input_handler() {}

  static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto &me = instance();

    auto found = me.m_tracked_keys.find(key);
    if (found == me.m_tracked_keys.end()) return;

    auto &btn_info = found->second;

    if (action == GLFW_PRESS) {
      btn_info.current_state = button_state::e_held_down;
    } else if (action == GLFW_RELEASE) {
      btn_info.current_state = button_state::e_pressed;
    }
  }

public:
  static input_handler &instance() {
    static input_handler handler;
    return handler;
  }

public:
  void monitor(key_index key, button_state state_to_notify) {
    m_tracked_keys[key] = tracked_key_info{button_state::e_idle, state_to_notify};
  }

  static void bind(GLFWwindow *window) { glfwSetKeyCallback(window, key_callback); }

  std::unordered_map<key_index, button_state> poll() {
    std::unordered_map<key_index, button_state> result;

    for (auto &v : m_tracked_keys) {
      if (v.second.current_state != v.second.look_for) continue;

      result.insert({v.first, v.second.current_state});
      // After polling the button change pressed to idle flag
      if (v.second.current_state == button_state::e_pressed) v.second.current_state = button_state::e_idle;
    }

    return result;
  }
};

class application final {
private:
  static constexpr uint32_t c_max_frames_in_flight = 2; // Double buffering
  static constexpr uint32_t c_graphics_queue_index = 0;
  static constexpr uint32_t c_present_queue_index = 0;
  static constexpr uint32_t c_dear_imgui_queue_index = 1;

private:
  applicaton_platform m_platform;

  // Logical device and queues needed for rendering
  ezvk::logical_device                             m_l_device;
  std::unique_ptr<ezvk::i_graphics_present_queues> m_graphics_present;

  vk::raii::CommandPool m_command_pool = nullptr;
  ezvk::upload_context  m_oneshot_upload;

  ezvk::swapchain      m_swapchain;
  ezvk::device_buffers m_uniform_buffers;

  throttle::graphics::descriptor_set_data        m_descriptor_set_data = nullptr;
  throttle::graphics::pipeline_data<vertex_type> m_pipeline_data = nullptr;

  ezvk::framebuffers  m_framebuffers;
  ezvk::device_buffer m_vertex_buffer;

  struct frame_rendering_info {
    vk::raii::Semaphore     image_availible_semaphore, render_finished_semaphore;
    vk::raii::Fence         in_flight_fence;
    vk::raii::CommandBuffer cmd = nullptr;
  };

  std::vector<frame_rendering_info>                  m_rendering_info;
  std::chrono::time_point<std::chrono::system_clock> m_prev_frame_start;

  std::size_t m_curr_frame = 0, m_verices_n = 0;
  camera      m_camera;

  static constexpr float c_velocity = 10.0f;
  static constexpr float c_angular_velocity = glm::radians(30.0f);

public:
  bool m_triangles_loaded = false;

  application(applicaton_platform platform) : m_platform{std::move(platform)} {
    initialize_logical_device_queues();

    // Create command pool and a context for submitting immediate copy operations (graphics queue family implicitly
    // supports copy operations)
    m_command_pool = {ezvk::create_command_pool(m_l_device(), m_graphics_present->graphics().family_index(), true)};

    m_oneshot_upload = ezvk::upload_context{&m_l_device(), &m_graphics_present->graphics(), &m_command_pool};

    m_swapchain = {m_platform.p_device(), m_l_device(), m_platform.surface(), m_platform.window().extent(),
                   m_graphics_present.get()};

    m_uniform_buffers = {c_max_frames_in_flight, sizeof(triangles::uniform_buffer_object), m_platform.p_device(),
                         m_l_device(), vk::BufferUsageFlagBits::eUniformBuffer};

    m_descriptor_set_data = {m_l_device(), m_uniform_buffers};

    m_pipeline_data = {m_l_device(), "shaders/vertex.spv", "shaders/fragment.spv", m_descriptor_set_data};
    m_framebuffers = {m_l_device(), m_swapchain.image_views(), m_swapchain.extent(), m_pipeline_data.m_render_pass};

    initialize_frame_rendering_info();
    initialize_input_hanlder();
  }

  void init() { m_prev_frame_start = std::chrono::system_clock::now(); }

  void loop() {
    render_frame();

    auto  finish = std::chrono::system_clock::now();
    float delta_t = std::chrono::duration<float>{finish - m_prev_frame_start}.count();

    auto &handler = input_handler::instance();
    auto  events = handler.poll();

    auto fwd_movement = (1.0f * events.count(GLFW_KEY_W)) - (1.0f * events.count(GLFW_KEY_S));
    auto side_movement = (1.0f * events.count(GLFW_KEY_D)) - (1.0f * events.count(GLFW_KEY_A));

    glm::vec3 dir_movement = fwd_movement * m_camera.get_direction() + side_movement * m_camera.get_sideways();

    if (throttle::geometry::is_definitely_greater(glm::length(dir_movement), 0.0f)) {
      m_camera.translate(glm::normalize(dir_movement) * c_velocity * delta_t);
    }

    auto yaw_movement = (1.0f * events.count(GLFW_KEY_RIGHT)) - (1.0f * events.count(GLFW_KEY_LEFT));
    auto pitch_movement = (1.0f * events.count(GLFW_KEY_DOWN)) - (1.0f * events.count(GLFW_KEY_UP));
    auto roll_movement = (1.0f * events.count(GLFW_KEY_Q)) - (1.0f * events.count(GLFW_KEY_E));

    glm::quat yaw_rotation = glm::angleAxis(yaw_movement * c_angular_velocity * delta_t, m_camera.get_up());
    glm::quat pitch_rotation = glm::angleAxis(pitch_movement * c_angular_velocity * delta_t, m_camera.get_sideways());
    glm::quat roll_rotation = glm::angleAxis(roll_movement * c_angular_velocity * delta_t, m_camera.get_direction());

    glm::quat resulting_rotation = yaw_rotation * pitch_rotation * roll_rotation;
    m_camera.rotate(resulting_rotation);

    m_prev_frame_start = std::chrono::system_clock::now();
  }

  auto *window() const { return m_platform.window()(); }

  void load_triangles(const std::vector<vertex_type> &vertices) {
    m_verices_n = vertices.size();

    auto size = ezvk::utils::sizeof_container(vertices);

    ezvk::device_buffer staging_buffer = {
        m_platform.p_device(), m_l_device(), vk::BufferUsageFlagBits::eTransferSrc, std::span{vertices},
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

    m_vertex_buffer = {m_platform.p_device(), m_l_device(), size,
                       vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                       vk::MemoryPropertyFlagBits::eDeviceLocal};

    auto &src_buffer = staging_buffer.buffer();
    auto &dst_buffer = m_vertex_buffer.buffer();

    m_oneshot_upload.immediate_submit([size, &src_buffer, &dst_buffer](vk::raii::CommandBuffer &cmd) {
      vk::BufferCopy copy = {0, 0, size};
      cmd.copyBuffer(*src_buffer, *dst_buffer, copy);
    });

    m_triangles_loaded = true;
  }

  void shutdown() { m_l_device().waitIdle(); }

private:
  void initialize_input_hanlder() {
    auto &handler = input_handler::instance();

    // Movements forward, backward and sideways
    handler.monitor(GLFW_KEY_W, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_A, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_S, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_D, input_handler::button_state::e_held_down);

    // Rotate around the camera direction axis
    handler.monitor(GLFW_KEY_Q, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_E, input_handler::button_state::e_held_down);

    // Rotate around the yaw and pitch axis
    handler.monitor(GLFW_KEY_RIGHT, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_LEFT, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_UP, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_DOWN, input_handler::button_state::e_held_down);

    handler.bind(m_platform.window()());
  }

  void initialize_logical_device_queues() {
    auto graphics_queue_indices = ezvk::find_graphics_family_indices(m_platform.p_device());
    auto present_queue_indices = ezvk::find_present_family_indices(m_platform.p_device(), m_platform.surface());

    std::vector<ezvk::queue_family_index_type> intersection;
    std::set_intersection(graphics_queue_indices.begin(), graphics_queue_indices.end(), present_queue_indices.begin(),
                          present_queue_indices.end(), std::back_inserter(intersection));

    const float default_priority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> reqs;

    ezvk::queue_family_index_type chosen_graphics, chosen_present;
    if (intersection.empty()) {
      chosen_graphics = graphics_queue_indices.front(); // Maybe find a queue family with maximum number of queues
      chosen_present = present_queue_indices.front();
      reqs.push_back({.queueFamilyIndex = chosen_graphics, .queueCount = 1, .pQueuePriorities = &default_priority});
      reqs.push_back({.queueFamilyIndex = chosen_present, .queueCount = 1, .pQueuePriorities = &default_priority});
    }

    else {
      chosen_graphics = chosen_present = intersection.front();
      reqs.push_back({.queueFamilyIndex = chosen_graphics, .queueCount = 1, .pQueuePriorities = &default_priority});
    }

    auto extensions = required_physical_device_extensions();
    m_l_device = ezvk::logical_device{m_platform.p_device(), reqs, extensions.begin(), extensions.end()};
    m_graphics_present = ezvk::make_graphics_present_queues(m_l_device(), chosen_graphics, c_graphics_queue_index,
                                                            chosen_present, c_present_queue_index);
  }

  void initialize_frame_rendering_info() {
    for (uint32_t i = 0; i < c_max_frames_in_flight; ++i) {
      frame_rendering_info primitive = {m_l_device().createSemaphore({}), m_l_device().createSemaphore({}),
                                        m_l_device().createFence({.flags = vk::FenceCreateFlagBits::eSignaled})};
      m_rendering_info.push_back(std::move(primitive));
    }
  }

  vk::raii::CommandBuffer fill_command_buffer(uint32_t image_index) {
    vk::CommandBufferAllocateInfo alloc_info = {
        .commandPool = *m_command_pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1u};
    vk::raii::CommandBuffer cmd = std::move(vk::raii::CommandBuffers{m_l_device(), alloc_info}.front());

    cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    vk::ClearValue          clear_color = {std::array<float, 4>{0.2f, 0.3f, 0.3f, 1.0f}};
    vk::RenderPassBeginInfo render_pass_info = {.renderPass = *m_pipeline_data.m_render_pass,
                                                .framebuffer = *m_framebuffers[image_index],
                                                .renderArea = {vk::Offset2D{0, 0}, m_swapchain.extent()},
                                                .clearValueCount = 1,
                                                .pClearValues = &clear_color};

    cmd.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    auto extent = m_swapchain.extent();

    vk::Viewport viewport = {0.0f,
                             static_cast<float>(extent.height),
                             static_cast<float>(extent.width),
                             -static_cast<float>(extent.height),
                             0.0f,
                             1.0f};

    cmd.setViewport(0, {viewport});
    cmd.setScissor(0, {{vk::Offset2D{0, 0}, extent}});

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline_data.m_pipeline);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipeline_data.m_layout, 0,
                           {*m_descriptor_set_data.m_descriptor_set}, nullptr);

    if (m_triangles_loaded) {
      cmd.bindVertexBuffers(0, *m_vertex_buffer.buffer(), {0});
      cmd.draw(m_verices_n, 1, 0, 0);
    }

    cmd.endRenderPass();
    cmd.end();

    return cmd;
  }

  void recreate_swap_chain() {
    auto extent = m_platform.window().extent();

    while (extent.width == 0 || extent.height == 0) {
      extent = m_platform.window().extent();
      glfwWaitEvents();
    }

    const auto &old_swapchain = m_swapchain();
    auto        new_swapchain = ezvk::swapchain{m_platform.p_device(),    m_l_device(),  m_platform.surface(), extent,
                                         m_graphics_present.get(), *old_swapchain};

    m_l_device().waitIdle();
    m_swapchain().clear(); // Destroy the old swapchain
    m_swapchain = std::move(new_swapchain);

    m_framebuffers = {m_l_device(), m_swapchain.image_views(), m_swapchain.extent(), m_pipeline_data.m_render_pass};
  }

  // INSPIRATION: https://github.com/tilir/cpp-graduate/blob/master/10-3d/vk-simplest.cc
  void render_frame() {
    auto &current_frame_data = m_rendering_info.at(m_curr_frame);
    m_l_device().waitForFences({*current_frame_data.in_flight_fence}, VK_TRUE, UINT64_MAX);

    vk::AcquireNextImageInfoKHR acquire_info = {.swapchain = *m_swapchain(),
                                                .timeout = UINT64_MAX,
                                                .semaphore = *current_frame_data.image_availible_semaphore,
                                                .fence = nullptr,
                                                .deviceMask = 1};

    uint32_t image_index;
    try {
      image_index = m_l_device().acquireNextImage2KHR(acquire_info).second;
    } catch (vk::OutOfDateKHRError &) {
      recreate_swap_chain();
      return;
    }

    current_frame_data.cmd = fill_command_buffer(image_index);

    m_uniform_buffers[m_curr_frame].copy_to_device(
        m_camera.get_vp_matrix(m_swapchain.extent().width, m_swapchain.extent().height));

    vk::PipelineStageFlags wait_stages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo submit_info = {.waitSemaphoreCount = 1,
                                  .pWaitSemaphores = std::addressof(*current_frame_data.image_availible_semaphore),
                                  .pWaitDstStageMask = std::addressof(wait_stages),
                                  .commandBufferCount = 1,
                                  .pCommandBuffers = &(*current_frame_data.cmd),
                                  .signalSemaphoreCount = 1,
                                  .pSignalSemaphores = std::addressof(*current_frame_data.render_finished_semaphore)};

    m_l_device().resetFences(*current_frame_data.in_flight_fence);
    m_graphics_present->graphics().queue().submit(submit_info, *current_frame_data.in_flight_fence);

    vk::PresentInfoKHR present_info = {.waitSemaphoreCount = 1,
                                       .pWaitSemaphores = std::addressof(*current_frame_data.render_finished_semaphore),
                                       .swapchainCount = 1,
                                       .pSwapchains = std::addressof(*m_swapchain()),
                                       .pImageIndices = &image_index};

    vk::Result result_present;
    try {
      result_present = m_graphics_present->present().queue().presentKHR(present_info);
    } catch (vk::OutOfDateKHRError &) {
      result_present = vk::Result::eErrorOutOfDateKHR;
    }

    if (result_present == vk::Result::eSuboptimalKHR || result_present == vk::Result::eErrorOutOfDateKHR) {
      recreate_swap_chain();
    }

    m_curr_frame = (m_curr_frame + 1) % c_max_frames_in_flight;
  }
};

} // namespace triangles