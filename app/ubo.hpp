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
#include <array>

namespace triangles {

struct uniform_buffer_object {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

} // namespace triangles