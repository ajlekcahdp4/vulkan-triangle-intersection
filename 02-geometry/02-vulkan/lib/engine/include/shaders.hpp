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

#include <vulkan/vulkan_raii.hpp>
namespace throttle {
namespace graphics {

inline std::vector<char> read_file(std::string filename) {

  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  std::cout << "Failed to load \"" << filename << "\"" << std::endl;

  size_t filesize{static_cast<size_t>(file.tellg())};

  std::vector<char> buffer(filesize);
  file.seekg(0);
  file.read(buffer.data(), filesize);

  file.close();
  return buffer;
}

} // namespace graphics
} // namespace throttle