/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "ezvk/instance.hpp"
#include "ezvk/window.hpp"

namespace triangles {

constexpr vk::PipelineRasterizationStateCreateInfo triangle_rasterization_state_create_info = {
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = vk::PolygonMode::eFill,
    .cullMode = vk::CullModeFlagBits::eFront,
    .frontFace = vk::FrontFace::eClockwise,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f,
};

constexpr vk::PipelineRasterizationStateCreateInfo wireframe_rasterization_state_create_info = {
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = vk::PolygonMode::eLine,
    .cullMode = vk::CullModeFlagBits::eNone,
    .frontFace = vk::FrontFace::eClockwise,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f,
};

inline auto required_vk_extensions() {
  auto glfw_extensions = ezvk::glfw_required_vk_extensions();
  glfw_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  return glfw_extensions;
}

inline std::vector<std::string> required_vk_layers(bool validation = false) {
  if (validation) return {"VK_LAYER_KHRONOS_validation"};
  return {};
}

inline std::vector<std::string> required_physical_device_extensions() { return {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; }

static constexpr std::array<float, 4> hex_to_rgba(uint32_t hex) {
  return {((hex >> 24) & 0xff) / 255.0f, ((hex >> 16) & 0xff) / 255.0f, ((hex >> 8) & 0xff) / 255.0f,
          ((hex >> 0) & 0xff) / 255.0f};
}

}; // namespace triangles