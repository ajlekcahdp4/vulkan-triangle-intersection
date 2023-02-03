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

#include "glm_inlcude.hpp"
#include "vulkan_hpp_include.hpp"

namespace triangles {

struct camera final {
private:
  glm::vec3 direction, up;
  float     fov, z_near_clip, z_far_clip;

public:
  glm::vec3 position;

  camera(glm::vec3 p_pos = {0, 0, 25}, glm::vec3 p_dir = {0, 0, -1.0f}, glm::vec3 p_up = {0, 1.0f, 0},
         float p_fov = glm::radians(45.0), float p_near_clip = 0.1f, float p_far_clip = 1000.0f)
      : position{p_pos}, direction{glm::normalize(p_dir)}, up{glm::normalize(p_up)}, fov{p_fov},
        z_near_clip{p_near_clip}, z_far_clip{p_far_clip} {}

  void  set_fov_degrees(float degrees) { fov = glm::radians(degrees); }
  float get_fov_degrees() const { return glm::degrees(fov); }

  void      translate(glm::vec3 translation) { position += translation; }
  glm::vec3 get_direction() const { return direction; }
  glm::vec3 get_up() const { return up; }
  glm::vec3 get_sideways() const { return glm::cross(direction, up); }

  // Only pass here a normalized quaternion
  void rotate(glm::quat q) {
    direction = direction * q; // GLM is so stupid. WHY, just why...
    up = up * q; // I can't wrap my brain around the fact that this is somehow the Adjunt of the vector up (should be q
                 // * up * ~q)
  }

  glm::mat4x4 get_vp_matrix(uint32_t width, uint32_t height) {
    auto view = glm::lookAt(position, position + direction, up);
    auto proj = glm::perspective(fov, static_cast<float>(width) / height, z_near_clip, z_far_clip);
    return proj * view;
  }
};

} // namespace triangles