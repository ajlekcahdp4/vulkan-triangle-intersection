/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "error.hpp"
#include "vulkan_include.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>
#include <string>

namespace ezvk {

class glfw_error : public ezvk::error {
  int m_error_code;

public:
  glfw_error(int ec, std::string msg) : ezvk::error{msg}, m_error_code{ec} {}

  auto error_code() const { return m_error_code; }
};

inline void check_glfw_error() {
  const char *description = nullptr;
  const auto  ec = glfwGetError(&description);
  if (ec != GLFW_NO_ERROR) throw ezvk::glfw_error{ec, description};
}

inline void default_glfw_error_callback(int ec, const char *msg) { throw ezvk::glfw_error{ec, msg}; }

inline void enable_glfw_exceptions() {
  glfwSetErrorCallback(default_glfw_error_callback);
  check_glfw_error();
}

class unique_glfw_window {
private:
  struct glfw_window_deleter {
    void operator()(GLFWwindow *ptr) { glfwDestroyWindow(ptr); }
  };

private:
  std::unique_ptr<GLFWwindow, glfw_window_deleter> m_handle;

public:
  unique_glfw_window(GLFWwindow *ptr) : m_handle{ptr} {}
  unique_glfw_window(const std::string &name, const vk::Extent2D &extent, bool resizable = false);

  vk::Extent2D size() const {
    int width, height;
    glfwGetFramebufferSize(m_handle.get(), &width, &height);
    return vk::Extent2D{.width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height)};
  }

  GLFWwindow *operator()() const { return m_handle.get(); }
};

class surface {
private:
  vk::raii::SurfaceKHR m_surface = nullptr;

public:
  surface(const vk::raii::Instance &instance, const unique_glfw_window &window);

  auto       &operator()() { return m_surface; }
  const auto &operator()() const { return m_surface; }
};

}; // namespace ezvk