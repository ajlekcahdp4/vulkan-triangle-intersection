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

#include <fstream>
#include <iostream>
#include <string>

#include <vulkan/vulkan_raii.hpp>
namespace throttle {
namespace graphics {

inline std::vector<char> read_file(std::string filename) {
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  file.open(filename, std::ios::binary);
  if (!file.is_open()) throw std::runtime_error("Failed to load file");
  std::istreambuf_iterator<char> start(file), fin;
  return std::vector<char>(start, fin);
}

inline vk::raii::ShaderModule create_module(const std::string &filename, const vk::raii::Device &p_device) {
  auto                       source_code = read_file(filename);
  vk::ShaderModuleCreateInfo module_info{};
  module_info.flags = vk::ShaderModuleCreateFlags{};
  module_info.codeSize = source_code.size();
  module_info.pCode = reinterpret_cast<const uint32_t *>(source_code.data());
  return vk::raii::ShaderModule(p_device, module_info);
}

} // namespace graphics
} // namespace throttle