set(ENGINE_SOURCES
  src/wrappers/surface.cc
  src/wrappers/window.cc
  src/wrappers/shaders.cc
  src/wrappers/instance.cc
  src/wrappers/device.cc
  src/wrappers/queue_families.cc
)

add_library(thengine ${ENGINE_SOURCES})
target_include_directories(thengine PUBLIC include)