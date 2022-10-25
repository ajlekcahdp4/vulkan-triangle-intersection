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
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                                              VkDebugUtilsMessageTypeFlagsEXT             message_types,
                                              VkDebugUtilsMessengerCallbackDataEXT const *call_back_data, void *) {

  std::cerr << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(message_severity)) << ": "
            << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(message_types)) << ":\n";
  std::cerr << std::string("\t") << "messageIDName   = <" << call_back_data->pMessageIdName << ">\n";
  std::cerr << std::string("\t") << "messageIdNumber = " << call_back_data->messageIdNumber << "\n";
  std::cerr << std::string("\t") << "message         = <" << call_back_data->pMessage << ">\n";
  if (0 < call_back_data->queueLabelCount) {
    std::cerr << std::string("\t") << "Queue Labels:\n";
    for (uint32_t i = 0; i < call_back_data->queueLabelCount; i++) {
      std::cerr << std::string("\t\t") << "labelName = <" << call_back_data->pQueueLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < call_back_data->cmdBufLabelCount) {
    std::cerr << std::string("\t") << "CommandBuffer Labels:\n";
    for (uint32_t i = 0; i < call_back_data->cmdBufLabelCount; i++) {
      std::cerr << std::string("\t\t") << "labelName = <" << call_back_data->pCmdBufLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < call_back_data->objectCount) {
    std::cerr << std::string("\t") << "Objects:\n";
    for (uint32_t i = 0; i < call_back_data->objectCount; i++) {
      std::cerr << std::string("\t\t") << "Object " << i << "\n";
      std::cerr << std::string("\t\t\t") << "objectType   = "
                << vk::to_string(static_cast<vk::ObjectType>(call_back_data->pObjects[i].objectType)) << "\n";
      std::cerr << std::string("\t\t\t") << "objectHandle = " << call_back_data->pObjects[i].objectHandle << "\n";
      if (call_back_data->pObjects[i].pObjectName) {
        std::cerr << std::string("\t\t\t") << "objectName   = <" << call_back_data->pObjects[i].pObjectName << ">\n";
      }
    }
  }
  return VK_TRUE;
}

vk::raii::DebugUtilsMessengerEXT create_debug_messenger(const vk::raii::Instance &instance) {
  vk::DebugUtilsMessengerCreateInfoEXT create_info{
      vk::DebugUtilsMessengerCreateFlagsEXT(),
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      debug_callback, nullptr};
  return instance.createDebugUtilsMessengerEXT(create_info);
}

} // namespace graphics
} // namespace throttle
