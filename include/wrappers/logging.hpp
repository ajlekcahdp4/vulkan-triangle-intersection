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

#include "vulkan_include.hpp"

#include <iostream>

namespace throttle::graphics {

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                                              VkDebugUtilsMessageTypeFlagsEXT             message_types,
                                              VkDebugUtilsMessengerCallbackDataEXT const *call_back_data, void *);

vk::raii::DebugUtilsMessengerEXT create_debug_messenger(const vk::raii::Instance &instance);

} // namespace throttle::graphics
