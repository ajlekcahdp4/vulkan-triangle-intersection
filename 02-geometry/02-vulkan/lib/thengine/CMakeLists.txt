set(ENGINE_SOURCES
  src/wrappers/surface.cc
  src/wrappers/window.cc
  src/wrappers/shaders.cc
  src/wrappers/instance.cc
  src/wrappers/device.cc
  src/wrappers/queue_families.cc
)

add_library(thengine ${ENGINE_SOURCES})
if (VALIDATION_LAYERS)
    message("Vulkan validation layers enabled " ${VALIDATION_LAYERS})
    target_compile_definitions(thengine PUBLIC ENABLE_VALIDATION_LAYERS)
endif()
target_include_directories(thengine PUBLIC include)