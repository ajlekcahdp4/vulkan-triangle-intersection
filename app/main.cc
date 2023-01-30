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
#include "glfw_include.hpp"
#include "vulkan_hpp_include.hpp"

#include "spdlog/cfg/env.h"
#include <spdlog/spdlog.h>

#include "application.hpp"

int main() {
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

  ezvk::instance instance = {ctx, app_info, extensions.begin(), extensions.end(), layers.begin(), layers.end()};

#ifdef USE_DEBUG_EXTENSION
  ezvk::debugged_instance debugged_instance = {std::move(instance)};
  triangles::application  app = {std::move(debugged_instance)};
#else
  triangles::application app = {std::move(instance)};
#endif

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
  app.run();
}