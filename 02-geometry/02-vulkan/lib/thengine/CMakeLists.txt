set(ENGINE_SOURCES
  src/engine/surface.cc
  src/engine/window.cc
)

add_library(thengine ${ENGINE_SOURCES})
target_include_directories(thengine PUBLIC include)