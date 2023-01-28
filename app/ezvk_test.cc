#include "ezvk/debug.hpp"
#include "ezvk/instance.hpp"

#include "misc.hpp"
#include <exception>
#include <iostream>
#include <stdexcept>

int main() try {
  glfwInit();

  auto ext = triangles::required_vk_extensions();
  auto layers = triangles::required_vk_layers();

  vk::raii::Context   ctx;
  vk::ApplicationInfo info = {.pApplicationName = "Hello, World!",
                              .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                              .apiVersion = VK_MAKE_VERSION(1, 0, 0),
                              .pEngineName = "Junk Inc.",
                              .engineVersion = VK_MAKE_VERSION(1, 0, 0)};

  ezvk::instance i = {ctx, info, ext.begin(), ext.end(), layers.begin(), layers.end()};

  ezvk::debug_messenger debug_msger{i()};
} catch (ezvk::unsupported_error &e) {
  std::cout << e.what() << "\n";
  for (const auto &v : e.missing()) {
    std::cout << v << "\n";
  }
} catch (std::exception &e) {
  std::cout << e.what() << "\n";
}