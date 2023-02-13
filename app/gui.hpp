/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "unified_includes/vulkan_hpp_include.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#include "ezvk/debug.hpp"
#include "ezvk/window.hpp"
#include "ezvk/wrappers/debugged_instance.hpp"
#include "ezvk/wrappers/depth_buffer.hpp"
#include "ezvk/wrappers/descriptor_set.hpp"
#include "ezvk/wrappers/device.hpp"
#include "ezvk/wrappers/instance.hpp"
#include "ezvk/wrappers/memory.hpp"
#include "ezvk/wrappers/queues.hpp"
#include "ezvk/wrappers/renderpass.hpp"
#include "ezvk/wrappers/shaders.hpp"
#include "ezvk/wrappers/swapchain.hpp"

#include <string>

namespace triangles::gui {

constexpr uint32_t default_descriptor_count = 1000;

constexpr std::array<vk::DescriptorPoolSize, 11> imgui_pool_sizes = {
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

constexpr vk::AttachmentDescription imgui_renderpass_attachment_description = {
    .flags = vk::AttachmentDescriptionFlags{},
    .format = vk::Format::eB8G8R8A8Unorm,
    .samples = vk::SampleCountFlagBits::e1,
    .loadOp = vk::AttachmentLoadOp::eLoad,
    .storeOp = vk::AttachmentStoreOp::eStore,
    .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
    .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
    .initialLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .finalLayout = vk::ImageLayout::ePresentSrcKHR};

constexpr std::array<vk::SubpassDependency, 1> imgui_subpass_dependency = {
    vk::SubpassDependency{.srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits::eNone,
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite}};

namespace detail {

class imgui_resources {
  vk::raii::DescriptorPool m_descriptor_pool = nullptr;
  ezvk::render_pass m_imgui_render_pass;
  vk::raii::CommandBuffers m_imgui_command_buffers = nullptr;
  ezvk::framebuffers m_imgui_framebuffers;
  bool m_initialized = false;
};

} // namespace detail

template <typename t_application> class imgui_related_data {
private:
  bool m_initialized = false;

  static void imgui_check_vk_error(VkResult res) {
    vk::Result hpp_result = vk::Result{res};
    std::string error_message = vk::to_string(hpp_result);
    vk::resultCheck(hpp_result, error_message.c_str());
  }

public:
  vk::raii::DescriptorPool m_descriptor_pool = nullptr;
  ezvk::render_pass m_imgui_render_pass;
  vk::raii::CommandBuffers m_imgui_command_buffers = nullptr;
  ezvk::framebuffers m_imgui_framebuffers;

  imgui_related_data() = default;

  imgui_related_data(t_application &app) {
    m_descriptor_pool = ezvk::create_descriptor_pool(app.m_l_device(), imgui_pool_sizes);

    vk::AttachmentReference color_attachment_ref = {
        .attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass = {.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref};

    std::array<vk::AttachmentDescription, 1> attachments{imgui_renderpass_attachment_description};
    m_imgui_render_pass = ezvk::render_pass(app.m_l_device(), subpass, attachments, imgui_subpass_dependency);

    vk::CommandBufferAllocateInfo alloc_info = {.commandPool = *app.m_command_pool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = t_application::c_max_frames_in_flight};

    m_imgui_command_buffers = vk::raii::CommandBuffers{app.m_l_device(), alloc_info};
    m_imgui_framebuffers = {
        app.m_l_device(), app.m_swapchain.image_views(), app.m_swapchain.extent(), m_imgui_render_pass()};

    IMGUI_CHECKVERSION(); // Verify that compiled imgui binary matches the header
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(app.m_platform.window()(), true);
    ImGui_ImplVulkan_InitInfo info = {.Instance = *app.m_platform.instance(),
        .PhysicalDevice = *app.m_platform.p_device(),
        .Device = *app.m_l_device(),
        .QueueFamily = app.m_graphics_present->graphics().family_index(),
        .Queue = *app.m_graphics_present->graphics().queue(),
        .PipelineCache = VK_NULL_HANDLE,
        .DescriptorPool = *m_descriptor_pool,
        .Subpass = 0,
        .MinImageCount = app.m_swapchain.min_image_count(),
        .ImageCount = static_cast<uint32_t>(app.m_swapchain.images().size()),
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .Allocator = nullptr,
        .CheckVkResultFn = imgui_related_data::imgui_check_vk_error};

    ImGui_ImplVulkan_Init(&info, *m_imgui_render_pass());
    // Here we should create a render pass specific to Dear ImGui

    // Upload font textures to the GPU via oneshot immediate submit
    app.m_oneshot_upload.immediate_submit(
        [](vk::raii::CommandBuffer &cmd) { ImGui_ImplVulkan_CreateFontsTexture(*cmd); });

    m_initialized = true;
  }

  imgui_related_data(const imgui_related_data &rhs) = delete;
  imgui_related_data &operator=(const imgui_related_data &rhs) = delete;

  void swap(imgui_related_data &rhs) {
    std::swap(m_descriptor_pool, rhs.m_descriptor_pool);
    std::swap(m_imgui_render_pass, rhs.m_imgui_render_pass);
    std::swap(m_imgui_command_buffers, rhs.m_imgui_command_buffers);
    std::swap(m_imgui_framebuffers, rhs.m_imgui_framebuffers);
    std::swap(m_initialized, rhs.m_initialized);
  }

  imgui_related_data(imgui_related_data &&rhs) { swap(rhs); }

  imgui_related_data &operator=(imgui_related_data &&rhs) {
    swap(rhs);
    return *this;
  }

  ~imgui_related_data() {
    if (!m_initialized) return;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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

  static void new_frame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  static void render_frame() { ImGui::Render(); }
};

} // namespace triangles::gui