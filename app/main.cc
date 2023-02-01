/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "ezvk/debug.hpp"
#include "ezvk/device.hpp"
#include "ezvk/error.hpp"
#include "ezvk/queues.hpp"
#include "ezvk/window.hpp"
#include "glfw_include.hpp"
#include "vulkan_hpp_include.hpp"

#include "spdlog/cfg/env.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iterator>
#include <spdlog/spdlog.h>

#include "application.hpp"

int main() {
  ezvk::enable_glfw_exceptions();
  spdlog::cfg::load_env_levels(); // Read logging level from environment variables
  glfwInit();

  static constexpr auto app_info = vk::ApplicationInfo{.pApplicationName = "Hello, World!",
                                                       .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                                       .pEngineName = "Junk Inc.",
                                                       .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                                       .apiVersion = VK_MAKE_VERSION(1, 0, 0)};

  vk::raii::Context ctx;

  auto extensions = triangles::required_vk_extensions();

#ifdef USE_DEBUG_EXTENSION
  auto layers = triangles::required_vk_layers(true);
#else
  auto                   layers = triangles::required_vk_layers();
#endif

  ezvk::instance raw_instance = {ctx, app_info, extensions.begin(), extensions.end(), layers.begin(), layers.end()};

#ifdef USE_DEBUG_EXTENSION
  ezvk::generic_instance instance = ezvk::debugged_instance{std::move(raw_instance)};
#else
  ezvk::generic_instance instance = std::move(raw_instance);
#endif

  auto physical_device_extensions = triangles::required_physical_device_extensions();
  auto suitable_physical_devices = ezvk::physical_device_selector::enumerate_suitable_physical_devices(
      instance(), physical_device_extensions.begin(), physical_device_extensions.end());

  if (suitable_physical_devices.empty()) {
    throw ezvk::vk_error{"No suitable physical devices found"};
  }

  auto p_device = std::move(suitable_physical_devices.front());
  auto window = ezvk::unique_glfw_window{"Triangles intersection", vk::Extent2D{800, 600}, true};
  auto surface = ezvk::surface{instance(), window};

#if 0
#endif

  triangles::applicaton_platform platform = {std::move(instance), std::move(window), std::move(surface),
                                             std::move(p_device)};

  triangles::application app = {std::move(platform)};

  // clang-format off
  app.load_triangles({
    // triangle 1
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    // triangle 2
    {{1.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{1.0f, -0.5f, -0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    // triangle 3
    {{0.0f, 1.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.7f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, -0.5f, -2.0f}, {0.0f, 1.0f, 0.0f}}
    });
  // clang-format on

  while (!glfwWindowShouldClose(app.window())) {
    glfwPollEvents();
    app.loop();
  }

  app.shutdown();
}