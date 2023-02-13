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

#include "camera.hpp"
#include "equal.hpp"
#include "misc.hpp"
#include "pipeline.hpp"
#include "ubo.hpp"
#include "utils.hpp"
#include "vertex.hpp"

#include "ezvk/debug.hpp"
#include "ezvk/debugged_instance.hpp"
#include "ezvk/depth_buffer.hpp"
#include "ezvk/descriptor_set.hpp"
#include "ezvk/device.hpp"
#include "ezvk/instance.hpp"
#include "ezvk/memory.hpp"
#include "ezvk/queues.hpp"
#include "ezvk/renderpass.hpp"
#include "ezvk/shaders.hpp"
#include "ezvk/swapchain.hpp"
#include "ezvk/window.hpp"

#include "camera.hpp"
#include "gui.hpp"
#include "keyboard_handler.hpp"

#include "glfw_include.hpp"
#include "glm_inlcude.hpp"
#include "vulkan_hpp_include.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#if defined(VK_VALIDATION_LAYER) || !defined(NDEBUG)
#define USE_DEBUG_EXTENSION
#endif

namespace triangles {

constexpr uint32_t intersect_index = 1u, regular_index = 0u, wiremesh_index = 2u, bbox_index = 3u;
struct input_data {
  std::span<const triangles::triangle_vertex_type> tr_vert;
  std::span<const triangles::wireframe_vertex_type> broad_vert, bbox_vert;
};

struct applicaton_platform {
  ezvk::generic_instance m_instance;
  ezvk::unique_glfw_window m_window = nullptr;
  ezvk::surface m_surface;

  vk::raii::PhysicalDevice m_p_device = nullptr;

public:
  applicaton_platform(ezvk::generic_instance instance, ezvk::unique_glfw_window window, ezvk::surface surface,
      vk::raii::PhysicalDevice p_device)
      : m_instance{std::move(instance)}, m_window{std::move(window)}, m_surface{std::move(surface)},
        m_p_device{std::move(p_device)} {}

  auto &instance() & { return m_instance(); }
  const auto &instance() const & { return m_instance(); }

  auto &window() & { return m_window; }
  const auto &window() const & { return m_window; }

  auto &surface() & { return m_surface(); }
  const auto &surface() const & { return m_surface(); }

  auto &p_device() & { return m_p_device; }
  const auto &p_device() const & { return m_p_device; }
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
  ezvk::logical_device m_l_device;
  std::unique_ptr<ezvk::i_graphics_present_queues> m_graphics_present;

  vk::raii::CommandPool m_command_pool = nullptr;
  ezvk::upload_context m_oneshot_upload;
  ezvk::swapchain m_swapchain;

  vk::raii::DescriptorPool m_descriptor_pool = nullptr;

  ezvk::descriptor_set m_descriptor_set;
  ezvk::device_buffers m_uniform_buffers;

  ezvk::render_pass m_primitives_render_pass;
  ezvk::pipeline_layout m_primitives_pipeline_layout;

  ezvk::depth_buffer m_depth_buffer;

  pipeline<triangle_vertex_type> m_triangle_pipeline;
  pipeline<wireframe_vertex_type> m_wireframe_pipeline;

  ezvk::framebuffers m_framebuffers;
  std::atomic_bool m_data_loaded = false;

  struct vertex_draw_info {
    ezvk::device_buffer buf;

    std::atomic_bool loaded = false, in_staging = false;
    uint32_t count = 0, size = 0;

    ezvk::device_buffer staging_buffer;

  public:
    operator bool() const { return loaded.load(); }
  };

  vertex_draw_info m_triangle_draw_info;
  vertex_draw_info m_wireframe_broad_draw_info;
  vertex_draw_info m_wireframe_bbox_draw_info;

  struct frame_rendering_info {
    vk::raii::Semaphore image_availible_semaphore, render_finished_semaphore;
    vk::raii::Fence in_flight_fence;
  };

  vk::raii::CommandBuffers m_primitives_command_buffers = nullptr;

  std::vector<frame_rendering_info> m_rendering_info;
  std::chrono::time_point<std::chrono::system_clock> m_prev_frame_start;

  std::size_t m_curr_frame = 0;
  utils3d::camera m_camera;

  bool m_mod_speed = false;
  bool m_first_frame = true;

  using array_color4 = std::array<float, 4>;
  struct {
    float linear_velocity_reg = 500.0f;
    float angular_velocity_reg = 30.0f;
    float linear_velocity_mod = 5000.0f;
    float render_distance = 30000.0f;
    float fov = 90.0f;

    float light_dir_yaw = 0.0f, light_dir_pitch = 0.0f;
    float ambient_strength = 0.1f;

    glm::vec4 light_dir;

    std::array<float, 4> light_color = hex_to_rgba(0xffffffff);
    std::array<float, 4> clear_color = hex_to_rgba(0x181818ff);

    std::array<array_color4, c_color_count> colors = {
        hex_to_rgba(0x89c4e1ff), hex_to_rgba(0xff4c29ff), hex_to_rgba(0x2f363aff), hex_to_rgba(0x338568ff)};

    bool draw_broad_phase = false, draw_bbox = false;
  } m_configurable_parameters;

private:
  using gui_type = gui::imgui_related_data<application>;
  friend class gui::imgui_related_data<application>;
  gui_type m_imgui_data;

  static constexpr std::array<vk::SubpassDependency, 1> depth_subpass_dependency = {vk::SubpassDependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask =
          vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
      .dstStageMask =
          vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite}};

private:
  application(applicaton_platform platform) : m_platform{std::move(platform)} {
    initialize_logical_device_queues();

    // Create command pool and a context for submitting immediate copy operations (graphics queue family implicitly
    // supports copy operations)
    m_command_pool = ezvk::create_command_pool(m_l_device(), m_graphics_present->graphics().family_index(), true);
    m_oneshot_upload = ezvk::upload_context{&m_l_device(), &m_graphics_present->graphics(), &m_command_pool};

    m_swapchain = {m_platform.p_device(), m_l_device(), m_platform.surface(), m_platform.window().extent(),
        m_graphics_present.get()};

    initialize_primitives_pipeline();
    initialize_input_hanlder(); // Bind key strokes

    initialize_frame_rendering_info(); // Initialize data needed to render primitives
    initialize_imgui();                // Initialize GUI specific objects
  }

private:
  class singleton_helper {
    std::unique_ptr<application> m_instance;

  public:
    application &get(applicaton_platform *platform = nullptr) {
      if (platform) {
        if (m_instance.get()) throw std::invalid_argument{"Application instance is already initialized"};
        m_instance = std::unique_ptr<application>{new application{std::move(*platform)}};
        return *m_instance.get();
      }

      if (!m_instance.get()) throw std::invalid_argument{"Application instance hasn't been initialized"};
      return *m_instance.get();
    }

    void destroy() { m_instance.reset(); }
  };

public:
  static singleton_helper &instance() {
    static singleton_helper helper;
    return helper;
  }

  void loop() {
    auto current_time = std::chrono::system_clock::now();

    if (!m_first_frame) {
      physics_loop(std::chrono::duration<float>{current_time - m_prev_frame_start}.count());
    } else {
      m_first_frame = false;
    }

    m_prev_frame_start = current_time;

    gui_type::new_frame();
    draw_gui();
    gui_type::render_frame();

    // Here we update the camera parameters
    m_camera.set_far_z_clip(m_configurable_parameters.render_distance);
    m_camera.set_fov_degrees(m_configurable_parameters.fov);

    render_frame();
  }

  auto *window() const { return m_platform.window()(); }

  void load_input_data(const input_data &data) {
    if (m_data_loaded.load()) throw std::invalid_argument{"For now you can't load vertex data more than once"};

    if (!data.tr_vert.empty()) load_draw_info(data.tr_vert, m_triangle_draw_info);
    if (!data.broad_vert.empty()) load_draw_info(data.broad_vert, m_wireframe_broad_draw_info);
    if (!data.bbox_vert.empty()) load_draw_info(data.bbox_vert, m_wireframe_bbox_draw_info);

    m_data_loaded.store(true);
  }

  void shutdown() { m_l_device().waitIdle(); }

private:
  ezvk::device_buffer copy_to_staging_memory(const auto &container) {
    assert(!container.empty());

    ezvk::device_buffer staging_buffer = {m_platform.p_device(), m_l_device(), vk::BufferUsageFlagBits::eTransferSrc,
        std::span{container}, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

    return staging_buffer;
  }

  void copy_to_device_memory(vk::raii::CommandBuffer &cmd, vertex_draw_info &info) {
    const auto size = info.size;

    info.buf = {m_platform.p_device(), m_l_device(), size,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal};

    auto &src_buffer = info.staging_buffer.buffer();
    auto &dst_buffer = info.buf.buffer();

    vk::BufferCopy copy = {0, 0, size};
    cmd.copyBuffer(*src_buffer, *dst_buffer, copy);

    vk::BufferMemoryBarrier barrier = {.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
        .dstAccessMask = vk::AccessFlagBits::eVertexAttributeRead,
        .buffer = *dst_buffer,
        .offset = 0,
        .size = info.size};

    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eVertexInput, {}, nullptr, barrier, nullptr);
  }

  void load_draw_info(const auto &vertices, vertex_draw_info &info) {
    assert(!vertices.empty());
    info.count = vertices.size();
    info.size = ezvk::utils::sizeof_container(vertices);
    info.staging_buffer = copy_to_staging_memory(vertices);
    info.in_staging.store(true);
  }

  void physics_loop(float delta) {
    auto &handler = ezio::keyboard_handler::instance();
    auto events = handler.poll();

    if (ImGui::GetIO().WantCaptureKeyboard) return;

    if (events.contains(GLFW_KEY_LEFT_SHIFT)) m_mod_speed = !m_mod_speed;

    const auto calculate_movement = [events](int plus, int minus) {
      return (1.0f * events.count(plus)) - (1.0f * events.count(minus));
    };

    auto fwd_movement = calculate_movement(GLFW_KEY_W, GLFW_KEY_S);
    auto side_movement = calculate_movement(GLFW_KEY_D, GLFW_KEY_A);
    auto up_movement = calculate_movement(GLFW_KEY_SPACE, GLFW_KEY_C);

    glm::vec3 dir_movement = fwd_movement * m_camera.get_direction() + side_movement * m_camera.get_sideways() +
        up_movement * m_camera.get_up();

    float speed =
        (m_mod_speed ? m_configurable_parameters.linear_velocity_mod : m_configurable_parameters.linear_velocity_reg);
    if (throttle::geometry::is_definitely_greater(glm::length(dir_movement), 0.0f)) {
      m_camera.translate(glm::normalize(dir_movement) * speed * delta);
    }

    auto yaw_movement = calculate_movement(GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
    auto pitch_movement = calculate_movement(GLFW_KEY_DOWN, GLFW_KEY_UP);
    auto roll_movement = calculate_movement(GLFW_KEY_Q, GLFW_KEY_E);

    auto angular_per_delta_t = glm::radians(m_configurable_parameters.angular_velocity_reg) * delta;

    glm::quat yaw_rotation = glm::angleAxis(yaw_movement * angular_per_delta_t, m_camera.get_up());
    glm::quat pitch_rotation = glm::angleAxis(pitch_movement * angular_per_delta_t, m_camera.get_sideways());
    glm::quat roll_rotation = glm::angleAxis(roll_movement * angular_per_delta_t, m_camera.get_direction());

    glm::quat resulting_rotation = yaw_rotation * pitch_rotation * roll_rotation;
    m_camera.rotate(resulting_rotation);
  }

  struct gui_runtime_persistent_state {
    bool metrics_window_open = false;
  } m_gui_runtime;

  void draw_gui() {
    if (m_gui_runtime.metrics_window_open) ImGui::ShowMetricsWindow(&m_gui_runtime.metrics_window_open);

    ImGui::Begin("Triangles with Vulkan");

    if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("Move the camera:");
      ImGui::BulletText("Forwards/Backwards with W/S");
      ImGui::BulletText("Sideways to the Left/Right with A/D");
      ImGui::BulletText("Up/Down with Space/C");

      ImGui::Text("Rotate the camera:");
      ImGui::BulletText("Yaw with Left/Right Arrows");
      ImGui::BulletText("Pitch with Up/Down Arrows");
      ImGui::BulletText("Roll with Q/E");

      ImGui::Text("Press Left Shift to change between regular/fast speed");
    }

    if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
      ImGui::DragFloat("Linear velocity (regular)", &m_configurable_parameters.linear_velocity_reg, 1.0f);
      ImGui::DragFloat("Linear velocity (mod)", &m_configurable_parameters.linear_velocity_mod, 10.0f);
      ImGui::DragFloat("Angular velocity", &m_configurable_parameters.angular_velocity_reg, 0.1f);
      ImGui::PopItemWidth();
    }

    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
      ImGui::DragFloat("Rendering distance", &m_configurable_parameters.render_distance, 50.0f);
      ImGui::DragFloat(
          "Fov", &m_configurable_parameters.fov, 0.1f, 45.0f, 175.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

      ImGui::Checkbox("Visualize broad phase", &m_configurable_parameters.draw_broad_phase);
      ImGui::Checkbox("Draw bounding boxes", &m_configurable_parameters.draw_bbox);

      ImGui::BulletText("Color configuration");

      ImGui::ColorEdit4("Regular", m_configurable_parameters.colors[regular_index].data());
      ImGui::ColorEdit4("Intersecting", m_configurable_parameters.colors[intersect_index].data());
      ImGui::ColorEdit4("Wiremesh", m_configurable_parameters.colors[wiremesh_index].data());
      ImGui::ColorEdit4("Bounding box", m_configurable_parameters.colors[bbox_index].data());
      ImGui::ColorEdit4("Clear Color", m_configurable_parameters.clear_color.data());

      if (ImGui::Button("Open Metrics/Debug Window")) {
        m_gui_runtime.metrics_window_open = true;
      }

      ImGui::PopItemWidth();
    }

    if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);

      ImGui::DragFloat("Ambient Strength", &m_configurable_parameters.ambient_strength, 0.001f, 0.0f, 1.0f, "%.3f",
          ImGuiSliderFlags_AlwaysClamp);

      ImGui::ColorEdit4("Light Color", m_configurable_parameters.light_color.data());

      ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.35f);
      ImGui::BulletText("Light direction");
      ImGui::DragFloat(
          "Yaw", &m_configurable_parameters.light_dir_yaw, 0.1f, 0.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      ImGui::SameLine();
      ImGui::DragFloat("Pitch", &m_configurable_parameters.light_dir_pitch, 0.1f, 0.0f, 360.0f, "%.3f",
          ImGuiSliderFlags_AlwaysClamp);

      m_configurable_parameters.light_dir = glm::eulerAngleYX(glm::radians(m_configurable_parameters.light_dir_yaw),
                                                glm::radians(m_configurable_parameters.light_dir_pitch)) *
          glm::vec4{0, 0, 1, 0};

      auto light_dir = m_configurable_parameters.light_dir;
      ImGui::Text("Light direction: x = %.3f, y = %.3f, z = %.3f", light_dir.x, light_dir.y, light_dir.z);

      ImGui::PopItemWidth();
      ImGui::PopItemWidth();
    }

    ImGui::End();
  }

  static constexpr std::array<vk::DescriptorPoolSize, 1> c_global_descriptor_pool_sizes = {
      {{vk::DescriptorType::eUniformBuffer, 16}}};

  // We use two pipelines with the same descriptor set, so we should allocate a descriptor set with 2 binding points for
  // a uniform buffer.
  static constexpr std::array<ezvk::descriptor_set::binding_description, 2> descriptor_set_bindings = {
      {{vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics},
          {vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics}}};

  static constexpr vk::PipelineRasterizationStateCreateInfo triangle_rasterization_state_create_info = {
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eFront,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };

  static constexpr vk::PipelineRasterizationStateCreateInfo wireframe_rasterization_state_create_info = {
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = vk::PolygonMode::eLine,
      .cullMode = vk::CullModeFlagBits::eNone,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };

  void initialize_primitives_pipeline() {
    m_descriptor_pool = ezvk::create_descriptor_pool(m_l_device(), c_global_descriptor_pool_sizes);

    m_uniform_buffers = {c_max_frames_in_flight, sizeof(triangles::ubo), m_platform.p_device(), m_l_device(),
        vk::BufferUsageFlagBits::eUniformBuffer};

    m_descriptor_set = {m_l_device(), m_uniform_buffers, m_descriptor_pool, descriptor_set_bindings};

    // clang-format off
    constexpr vk::AttachmentReference 
      color_attachment_ref = {.attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal}, 
      depth_attachment_ref = {.attachment = 1, .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal};
    // clang-format on

    vk::SubpassDescription subpass = {.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref};

    const vk::Format depth_format = ezvk::find_depth_format(m_platform.p_device()).at(0);

    std::array<vk::AttachmentDescription, 2> attachments = {
        primitives_renderpass_attachment_description, ezvk::create_depth_attachment(depth_format)};

    m_primitives_render_pass = ezvk::render_pass{m_l_device(), subpass, attachments, depth_subpass_dependency};
    m_depth_buffer = {m_platform.m_p_device, m_l_device(), depth_format, m_swapchain.extent()};
    m_primitives_pipeline_layout = {m_l_device(), m_descriptor_set.m_layout};

    m_triangle_pipeline = {m_l_device(), "shaders/triangles_vert.spv", "shaders/triangles_frag.spv",
        m_primitives_pipeline_layout(), m_primitives_render_pass(), triangle_rasterization_state_create_info,
        vk::PrimitiveTopology::eTriangleList};

    m_wireframe_pipeline = {m_l_device(), "shaders/wireframe_vert.spv", "shaders/wireframe_frag.spv",
        m_primitives_pipeline_layout(), m_primitives_render_pass(), wireframe_rasterization_state_create_info,
        vk::PrimitiveTopology::eLineList};

    m_framebuffers = {m_l_device(), m_swapchain.image_views(), m_swapchain.extent(), m_primitives_render_pass(),
        m_depth_buffer.m_image_view()};
  }

  void initialize_input_hanlder() {
    using ezio::keyboard_handler;
    using button_state = keyboard_handler::button_state;

    auto &handler = keyboard_handler::instance();
    const std::array<std::pair<keyboard_handler::key_index, keyboard_handler::button_state>, 13> keys = {
        {{GLFW_KEY_W, button_state::e_held_down}, {GLFW_KEY_A, button_state::e_held_down},
            {GLFW_KEY_S, button_state::e_held_down}, {GLFW_KEY_D, button_state::e_held_down},
            {GLFW_KEY_SPACE, button_state::e_held_down}, {GLFW_KEY_C, button_state::e_held_down},
            {GLFW_KEY_Q, button_state::e_held_down}, {GLFW_KEY_E, button_state::e_held_down},
            {GLFW_KEY_RIGHT, button_state::e_held_down}, {GLFW_KEY_LEFT, button_state::e_held_down},
            {GLFW_KEY_UP, button_state::e_held_down}, {GLFW_KEY_DOWN, button_state::e_held_down},
            {GLFW_KEY_LEFT_SHIFT, button_state::e_pressed}}};

    handler.monitor(keys.begin(), keys.end());
    handler.bind(m_platform.window()());
  }

  void initialize_imgui() {
    m_imgui_data = gui_type{*this};
    ImGui::StyleColorsDark();
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
    m_graphics_present = ezvk::make_graphics_present_queues(
        m_l_device(), chosen_graphics, c_graphics_queue_index, chosen_present, c_present_queue_index);
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

  void fill_command_buffer(vk::raii::CommandBuffer &cmd, uint32_t image_index, vk::Extent2D extent) {
    cmd.reset();
    cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    const auto submit_copy = [&](auto &info) -> void {
      if (!info.in_staging) return;
      copy_to_device_memory(cmd, info);
      info.in_staging.store(false);
      info.loaded.store(true);
    };

    submit_copy(m_triangle_draw_info);
    submit_copy(m_wireframe_bbox_draw_info);
    submit_copy(m_wireframe_broad_draw_info);

    std::array<vk::ClearValue, 2> clear_values;
    clear_values[0].color = m_configurable_parameters.clear_color;
    clear_values[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderPassBeginInfo render_pass_info = {.renderPass = *m_primitives_render_pass(),
        .framebuffer = *m_framebuffers[image_index],
        .renderArea = {vk::Offset2D{0, 0}, extent},
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data()};

    cmd.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    vk::Viewport viewport = {0.0f, static_cast<float>(extent.height), static_cast<float>(extent.width),
        -static_cast<float>(extent.height), 0.0f, 1.0f};

    cmd.setViewport(0, {viewport});
    cmd.setScissor(0, {{vk::Offset2D{0, 0}, extent}});

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_primitives_pipeline_layout(), 0,
        {*m_descriptor_set.m_descriptor_set}, nullptr);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_triangle_pipeline());

    const auto submit_draw_info = [&cmd](const auto &info) -> void {
      if (!info) return;
      cmd.bindVertexBuffers(0, *info.buf.buffer(), {0});
      cmd.draw(info.count, 1, 0, 0);
    };

    submit_draw_info(m_triangle_draw_info);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_wireframe_pipeline());

    if (m_configurable_parameters.draw_broad_phase) {
      submit_draw_info(m_wireframe_broad_draw_info);
    }

    if (m_configurable_parameters.draw_bbox) {
      submit_draw_info(m_wireframe_bbox_draw_info);
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

    auto new_swapchain = ezvk::swapchain{
        m_platform.p_device(), m_l_device(), m_platform.surface(), extent, m_graphics_present.get(), *old_swapchain};

    m_l_device().waitIdle();
    m_swapchain().clear(); // Destroy the old swapchain
    m_swapchain = std::move(new_swapchain);

    // Minimum number of images may have changed during swapchain recreation
    ImGui_ImplVulkan_SetMinImageCount(m_swapchain.min_image_count());
    m_depth_buffer = {m_platform.m_p_device, m_l_device(), m_depth_buffer.depth_format(), m_swapchain.extent()};
    m_framebuffers = {m_l_device(), m_swapchain.image_views(), m_swapchain.extent(), m_primitives_render_pass(),
        m_depth_buffer.m_image_view()};
    m_imgui_data.m_imgui_framebuffers = {
        m_l_device(), m_swapchain.image_views(), m_swapchain.extent(), m_imgui_data.m_imgui_render_pass()};
  }

  // INSPIRATION: https://github.com/tilir/cpp-graduate/blob/master/10-3d/vk-simplest.cc
  void render_frame() {
    auto &current_frame_data = m_rendering_info.at(m_curr_frame);
    static_cast<void>(m_l_device().waitForFences({*current_frame_data.in_flight_fence}, VK_TRUE, UINT64_MAX));
    // Static cast to silence warning

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
    m_imgui_data.fill_command_buffer(
        m_imgui_data.m_imgui_command_buffers[m_curr_frame], image_index, m_swapchain.extent());

    std::array<vk::CommandBuffer, 2> cmds = {
        *m_primitives_command_buffers[m_curr_frame], *m_imgui_data.m_imgui_command_buffers[m_curr_frame]};

    ubo uniform_buffer = {m_camera.get_vp_matrix(m_swapchain.extent().width, m_swapchain.extent().height), {},
        glm_vec_from_array(m_configurable_parameters.light_color), m_configurable_parameters.light_dir,
        m_configurable_parameters.ambient_strength};

    std::transform(m_configurable_parameters.colors.begin(), m_configurable_parameters.colors.end(),
        uniform_buffer.colors.begin(), [](auto a) { return glm_vec_from_array(a); });

    m_uniform_buffers[m_curr_frame].copy_to_device(uniform_buffer);

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