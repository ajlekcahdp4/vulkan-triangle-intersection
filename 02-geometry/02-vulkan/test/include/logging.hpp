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
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace throttle {
namespace graphics {
VKAPI_ATTR VkBool32 VKAPI_CALL debug_call_back(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                               VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                               void                                       *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

vk::raii::DebugUtilsMessengerEXT create_debug_messenger(const vk::raii::Instance &instance) {
  vk::DebugUtilsMessengerCreateInfoEXT create_info{
      vk::DebugUtilsMessengerCreateFlagsEXT(),
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      debug_call_back, nullptr};
  return instance.createDebugUtilsMessengerEXT(create_info);
}

} // namespace graphics
} // namespace throttle
