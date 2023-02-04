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

#include "camera.hpp"
#include "misc.hpp"
#include "pipeline.hpp"
#include "ubo.hpp"
#include "utils.hpp"
#include "vertex.hpp"

#include "ezvk/debug.hpp"
#include "ezvk/debugged_instance.hpp"
#include "ezvk/device.hpp"
#include "ezvk/instance.hpp"
#include "ezvk/memory.hpp"
#include "ezvk/queues.hpp"
#include "ezvk/renderpass.hpp"
#include "ezvk/shaders.hpp"
#include "ezvk/swapchain.hpp"
#include "ezvk/window.hpp"

#include "glfw_include.hpp"
#include "glm_inlcude.hpp"
#include "vulkan_hpp_include.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

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

  static void key_callback(GLFWwindow *, int key, int, int action, int) {
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

  static constexpr vk::AttachmentDescription primitives_renderpass_attachment_description = {
      .flags = vk::AttachmentDescriptionFlags{},
      .format = vk::Format::eB8G8R8A8Unorm,
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::eColorAttachmentOptimal};

private:
  applicaton_platform m_platform;

  // Logical device and queues needed for rendering
  ezvk::logical_device                             m_l_device;
  std::unique_ptr<ezvk::i_graphics_present_queues> m_graphics_present;

  vk::raii::CommandPool m_command_pool = nullptr;
  ezvk::upload_context  m_oneshot_upload;

  ezvk::swapchain      m_swapchain;
  ezvk::device_buffers m_uniform_buffers;

  ezvk::descriptor_set m_descriptor_set;

  ezvk::render_pass      m_primitives_render_pass;
  ezvk::pipeline_layout  m_primitives_pipeline_layout;
  triangle_pipeline_data m_triangle_pipeline;

  ezvk::framebuffers  m_framebuffers;
  ezvk::device_buffer m_vertex_buffer;

  struct frame_rendering_info {
    vk::raii::Semaphore image_availible_semaphore, render_finished_semaphore;
    vk::raii::Fence     in_flight_fence;
  };

  vk::raii::CommandBuffers m_primitives_command_buffers = nullptr;

  std::vector<frame_rendering_info>                  m_rendering_info;
  std::chrono::time_point<std::chrono::system_clock> m_prev_frame_start;

  std::size_t m_curr_frame = 0, m_verices_n = 0;
  camera      m_camera;

  friend class imgui_related_data;
  struct imgui_related_data {
    vk::raii::DescriptorPool m_descriptor_pool = nullptr;
    ezvk::render_pass        m_imgui_render_pass;
    vk::raii::CommandBuffers m_imgui_command_buffers = nullptr;
    ezvk::framebuffers       m_imgui_framebuffers;
    application             *m_app;

    imgui_related_data() = default;

    static void imgui_check_vk_error(VkResult res) {
      vk::Result  hpp_result = vk::Result{res};
      std::string error_message = vk::to_string(hpp_result);
      vk::resultCheck(hpp_result, error_message.c_str());
    }

    static constexpr uint32_t                               default_descriptor_count = 1000;
    static constexpr std::array<vk::DescriptorPoolSize, 11> imgui_pool_sizes = {
        {{vk::DescriptorType::eSampler, default_descriptor_count},
         {vk::DescriptorType::eCombinedImageSampler, default_descriptor_count},
         {vk::DescriptorType::eSampledImage, default_descriptor_count},
         {vk::DescriptorType::eStorageImage, default_descriptor_count},
         {vk::DescriptorType::eUniformTexelBuffer, default_descriptor_count},
         {vk::DescriptorType::eStorageTexelBuffer, default_descriptor_count},
         {vk::DescriptorType::eUniformBuffer, default_descriptor_count},
         {vk::DescriptorType::eStorageBuffer, default_descriptor_count},
         {vk::DescriptorType::eUniformBufferDynamic, default_descriptor_count},
         {vk::DescriptorType::eStorageBufferDynamic, default_descriptor_count},
         {vk::DescriptorType::eInputAttachment, default_descriptor_count}}};

    static constexpr vk::AttachmentDescription imgui_renderpass_attachment_description = {
        .flags = vk::AttachmentDescriptionFlags{},
        .format = vk::Format::eB8G8R8A8Unorm,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR};

    static constexpr std::array<vk::SubpassDependency, 1> imgui_subpass_dependency = {
        vk::SubpassDependency{.srcSubpass = VK_SUBPASS_EXTERNAL,
                              .dstSubpass = 0,
                              .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                              .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
                              .srcAccessMask = vk::AccessFlagBits::eNone,
                              .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite}};

    imgui_related_data(application &app) {
      uint32_t max_sets = default_descriptor_count * imgui_pool_sizes.size();

      vk::DescriptorPoolCreateInfo descriptor_info = {.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                                      .maxSets = max_sets,
                                                      .poolSizeCount = static_cast<uint32_t>(imgui_pool_sizes.size()),
                                                      .pPoolSizes = imgui_pool_sizes.data()};

      m_descriptor_pool = vk::raii::DescriptorPool{app.m_l_device(), descriptor_info};
      m_imgui_render_pass = {app.m_l_device(), imgui_renderpass_attachment_description, imgui_subpass_dependency};

      vk::CommandBufferAllocateInfo alloc_info = {.commandPool = *app.m_command_pool,
                                                  .level = vk::CommandBufferLevel::ePrimary,
                                                  .commandBufferCount = c_max_frames_in_flight};

      m_imgui_command_buffers = vk::raii::CommandBuffers{app.m_l_device(), alloc_info};
      m_imgui_framebuffers = {app.m_l_device(), app.m_swapchain.image_views(), app.m_swapchain.extent(),
                              m_imgui_render_pass()};
    }

    void fill_command_buffer(vk::raii::CommandBuffer &cmd, uint32_t image_index, vk::Extent2D extent) {
      cmd.reset();
      cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

      vk::RenderPassBeginInfo render_pass_info = {.renderPass = *m_imgui_render_pass(),
                                                  .framebuffer = *m_imgui_framebuffers[image_index],
                                                  .renderArea = {vk::Offset2D{0, 0}, extent}};

      cmd.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

      ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);

      cmd.endRenderPass();
      cmd.end();
    }
  } m_imgui_data;

  bool m_triangles_loaded = false;

  static constexpr float c_velocity = 100.0f;
  static constexpr float c_angular_velocity = glm::radians(30.0f);

public:
  application(applicaton_platform platform) : m_platform{std::move(platform)} {
    initialize_logical_device_queues();

    // Create command pool and a context for submitting immediate copy operations (graphics queue family implicitly
    // supports copy operations)
    m_command_pool = ezvk::create_command_pool(m_l_device(), m_graphics_present->graphics().family_index(), true);
    m_oneshot_upload = ezvk::upload_context{&m_l_device(), &m_graphics_present->graphics(), &m_command_pool};

    m_swapchain = {m_platform.p_device(), m_l_device(), m_platform.surface(), m_platform.window().extent(),
                   m_graphics_present.get()};

    m_uniform_buffers = {c_max_frames_in_flight, sizeof(triangles::uniform_buffer_object), m_platform.p_device(),
                         m_l_device(), vk::BufferUsageFlagBits::eUniformBuffer};

    m_descriptor_set = {m_l_device(), m_uniform_buffers};

    m_primitives_render_pass = {m_l_device(), primitives_renderpass_attachment_description};
    m_primitives_pipeline_layout = {m_l_device(), m_descriptor_set.m_layout};
    m_triangle_pipeline = {m_l_device(), "shaders/vertex.spv", "shaders/fragment.spv", m_primitives_pipeline_layout(),
                           m_primitives_render_pass()};

    m_framebuffers = {m_l_device(), m_swapchain.image_views(), m_swapchain.extent(), m_primitives_render_pass()};

    initialize_input_hanlder(); // Bind key strokes

    initialize_frame_rendering_info(); // Initialize data needed to render primitives
    initilize_imgui();                 // Initialize GUI specific objects
  }

  void init() { m_prev_frame_start = std::chrono::system_clock::now(); }

  void loop() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    draw_gui();
    ImGui::Render();

    render_frame();

    auto  finish = std::chrono::system_clock::now();
    float delta_t = std::chrono::duration<float>{finish - m_prev_frame_start}.count();

    auto &handler = input_handler::instance();
    auto  events = handler.poll();

    const auto calculate_movement = [events](int plus, int minus) {
      return (1.0f * events.count(plus)) - (1.0f * events.count(minus));
    };

    auto fwd_movement = calculate_movement(GLFW_KEY_W, GLFW_KEY_S);
    auto side_movement = calculate_movement(GLFW_KEY_D, GLFW_KEY_A);
    auto up_movement = calculate_movement(GLFW_KEY_SPACE, GLFW_KEY_C);

    glm::vec3 dir_movement = fwd_movement * m_camera.get_direction() + side_movement * m_camera.get_sideways() +
                             up_movement * m_camera.get_up();

    if (throttle::geometry::is_definitely_greater(glm::length(dir_movement), 0.0f)) {
      m_camera.translate(glm::normalize(dir_movement) * c_velocity * delta_t);
    }

    auto yaw_movement = calculate_movement(GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
    auto pitch_movement = calculate_movement(GLFW_KEY_DOWN, GLFW_KEY_UP);
    auto roll_movement = calculate_movement(GLFW_KEY_Q, GLFW_KEY_E);

    auto angular_per_delta_t = c_angular_velocity * delta_t;

    glm::quat yaw_rotation = glm::angleAxis(yaw_movement * angular_per_delta_t, m_camera.get_up());
    glm::quat pitch_rotation = glm::angleAxis(pitch_movement * angular_per_delta_t, m_camera.get_sideways());
    glm::quat roll_rotation = glm::angleAxis(roll_movement * angular_per_delta_t, m_camera.get_direction());

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

  void shutdown() {
    m_l_device().waitIdle();
    ImGui_ImplVulkan_Shutdown(); // This should probably go in the destructor or in a custom class. For now this will do
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

private:
  void draw_gui() { ImGui::ShowDemoWindow(); }

  void initialize_input_hanlder() {
    auto &handler = input_handler::instance();

    // Movements forward, backward and sideways
    handler.monitor(GLFW_KEY_W, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_A, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_S, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_D, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_SPACE, input_handler::button_state::e_held_down);
    handler.monitor(GLFW_KEY_C, input_handler::button_state::e_held_down);

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

    vk::CommandBufferAllocateInfo alloc_info = {.commandPool = *m_command_pool,
                                                .level = vk::CommandBufferLevel::ePrimary,
                                                .commandBufferCount = c_max_frames_in_flight};

    m_primitives_command_buffers = vk::raii::CommandBuffers{m_l_device(), alloc_info};
  }

  void initilize_imgui() {
    m_imgui_data = {*this};

    IMGUI_CHECKVERSION(); // Verify that compiled imgui binary matches the header
    ImGui::CreateContext();
    ImGui::StyleColorsDark(); // Blessed dark mode

    ImGui_ImplGlfw_InitForVulkan(m_platform.window()(), true);
    ImGui_ImplVulkan_InitInfo info = {.Instance = *m_platform.instance(),
                                      .PhysicalDevice = *m_platform.p_device(),
                                      .Device = *m_l_device(),
                                      .QueueFamily = m_graphics_present->graphics().family_index(),
                                      .Queue = *m_graphics_present->graphics().queue(),
                                      .PipelineCache = VK_NULL_HANDLE,
                                      .DescriptorPool = *m_imgui_data.m_descriptor_pool,
                                      .Subpass = 0,
                                      .MinImageCount = m_swapchain.min_image_count(),
                                      .ImageCount = static_cast<uint32_t>(m_swapchain.images().size()),
                                      .CheckVkResultFn = imgui_related_data::imgui_check_vk_error};

    ImGui_ImplVulkan_Init(&info, *m_imgui_data.m_imgui_render_pass());
    // Here we should create a render pass specific to Dear ImGui
    m_oneshot_upload.immediate_submit([](vk::raii::CommandBuffer &cmd) { ImGui_ImplVulkan_CreateFontsTexture(*cmd); });
  }

  void fill_command_buffer(vk::raii::CommandBuffer &cmd, uint32_t image_index, vk::Extent2D extent) {
    cmd.reset();
    cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    vk::ClearValue          clear_color = {std::array<float, 4>{0.2f, 0.3f, 0.3f, 1.0f}};
    vk::RenderPassBeginInfo render_pass_info = {.renderPass = *m_primitives_render_pass(),
                                                .framebuffer = *m_framebuffers[image_index],
                                                .renderArea = {vk::Offset2D{0, 0}, extent},
                                                .clearValueCount = 1,
                                                .pClearValues = &clear_color};

    cmd.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    vk::Viewport viewport = {0.0f,
                             static_cast<float>(extent.height),
                             static_cast<float>(extent.width),
                             -static_cast<float>(extent.height),
                             0.0f,
                             1.0f};

    cmd.setViewport(0, {viewport});
    cmd.setScissor(0, {{vk::Offset2D{0, 0}, extent}});

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_triangle_pipeline());
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_primitives_pipeline_layout(), 0,
                           {*m_descriptor_set.m_descriptor_set}, nullptr);

    if (m_triangles_loaded) {
      cmd.bindVertexBuffers(0, *m_vertex_buffer.buffer(), {0});
      cmd.draw(m_verices_n, 1, 0, 0);
    }

    cmd.endRenderPass();
    cmd.end();
  }

  void recreate_swap_chain() {
    auto extent = m_platform.window().extent();

    while (extent.width == 0 || extent.height == 0) {
      extent = m_platform.window().extent();
      glfwWaitEvents();
    }

    const auto &old_swapchain = m_swapchain();

    auto new_swapchain = ezvk::swapchain{m_platform.p_device(),    m_l_device(),  m_platform.surface(), extent,
                                         m_graphics_present.get(), *old_swapchain};

    m_l_device().waitIdle();
    m_swapchain().clear(); // Destroy the old swapchain
    m_swapchain = std::move(new_swapchain);

    // Minimum number of images may have changed during swapchain recreation
    ImGui_ImplVulkan_SetMinImageCount(m_swapchain.min_image_count());
    m_framebuffers = {m_l_device(), m_swapchain.image_views(), m_swapchain.extent(), m_primitives_render_pass()};
    m_imgui_data.m_imgui_framebuffers = {m_l_device(), m_swapchain.image_views(), m_swapchain.extent(),
                                         m_imgui_data.m_imgui_render_pass()};
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

    fill_command_buffer(m_primitives_command_buffers[m_curr_frame], image_index, m_swapchain.extent());
    m_imgui_data.fill_command_buffer(m_imgui_data.m_imgui_command_buffers[m_curr_frame], image_index,
                                     m_swapchain.extent());

    std::array<vk::CommandBuffer, 2> cmds = {*m_primitives_command_buffers[m_curr_frame],
                                             *m_imgui_data.m_imgui_command_buffers[m_curr_frame]};

    m_uniform_buffers[m_curr_frame].copy_to_device(
        m_camera.get_vp_matrix(m_swapchain.extent().width, m_swapchain.extent().height));

    vk::PipelineStageFlags wait_stages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo submit_info = {.waitSemaphoreCount = 1,
                                  .pWaitSemaphores = std::addressof(*current_frame_data.image_availible_semaphore),
                                  .pWaitDstStageMask = std::addressof(wait_stages),
                                  .commandBufferCount = cmds.size(),
                                  .pCommandBuffers = cmds.data(),
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