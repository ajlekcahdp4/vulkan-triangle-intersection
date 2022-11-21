set(ENGINE_SOURCES
  src/wrappers/surface.cc
  src/wrappers/window.cc
  src/wrappers/shaders.cc
  src/wrappers/instance.cc
  src/wrappers/device.cc
  src/wrappers/queue_families.cc
  src/wrappers/pipeline.cc
  src/wrappers/swapchain.cc
  src/wrappers/logging.cc
  src/wrappers/memory.cc
)

add_library(thengine ${ENGINE_SOURCES})
if (VALIDATION_LAYERS)
    message("Vulkan validation layers enabled")
    target_compile_definitions(thengine PUBLIC ENABLE_VALIDATION_LAYERS)
endif()
target_include_directories(thengine PUBLIC include)