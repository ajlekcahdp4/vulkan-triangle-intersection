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

#include "glfw_include.hpp"
#include "vulkan_include.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace triangles {

class camera final {

private:
  // drecton should be normalized
  glm::vec3 direction = glm::normalize(glm::vec3{0.0f, 0.0f, -1.0f});

  glm::vec3 up = glm::normalize(glm::vec3{0.0f, 1.0f, 0.0f});

  float fov = glm::radians(45.0f);

  struct {
    glm::mat4x4 model = glm::mat4x4(1.0f);
    glm::mat4x4 view;
    glm::mat4x4 proj;
  } matrices;

public:
  glm::vec3 position = {0.0f, 0.0f, 25.0f};

  struct {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
  } keys;

  void set_up(const glm::vec3 &up_dir) { up = glm::normalize(up_dir); }

  void set_direction(const glm::vec3 &dir) { direction = glm::normalize(dir); }

  void set_fov(const float degrees) { fov = glm::radians(degrees); }

  void compute_matrices(const vk::Extent2D extent) {
    matrices.view = glm::lookAt(position, position + direction, up);
    matrices.proj = glm::perspective(fov, static_cast<float>(extent.width) / extent.height, 0.1f, 5000.0f);
  }

  glm::mat4x4 get_mvp_matrix(const vk::Extent2D extent) {
    compute_matrices(extent);
    return matrices.proj * matrices.view * matrices.model;
  }

  bool moving() { return keys.left || keys.right || keys.down || keys.up; }
};

} // namespace triangles