/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "utils.hpp"
#include "vulkan_hpp_include.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace throttle::graphics {

vk::raii::ShaderModule create_module(const std::string &filename, const vk::raii::Device &p_device) {
  auto source_code = ezvk::utils::read_file(filename);

  vk::ShaderModuleCreateInfo module_info = {.flags = vk::ShaderModuleCreateFlags{},
                                            .codeSize = source_code.size(),
                                            .pCode = reinterpret_cast<const uint32_t *>(source_code.data())};

  return vk::raii::ShaderModule{p_device, module_info};
}
}; // namespace throttle::graphics