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

#include "vulkan_hpp_include.hpp"

namespace throttle::graphics {

vk::raii::ShaderModule create_module(const std::string &filename, const vk::raii::Device &p_device);

} // namespace throttle::graphics