cmake_minimum_required(VERSION 3.11)
project(vulkan)

option(INSOURCEBUILD OFF)

if((${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR}) AND NOT ${INSOURCEBUILD})
  message(FATAL_ERROR "In-source building disabled. Provide -DINSOURCEBUILD option if you are sure about it.")
endif()

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

# Possibly support windows and MSVC compiler
if(MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra)
endif()

find_package(GTest)
set(NOGTEST OFF CACHE BOOL "Disable GoogleTest")

enable_testing()
if(NOT NOGTEST AND GTEST_FOUND AND CMAKE_MINOR_VERSION GREATER_EQUAL 20)
    set(ENABLE_GTEST ON)
else()
    message(WARNING "Google Test disabled")
endif()

set(NOLINT ON CACHE BOOL "Disable clang-tidy")

if(NOT ${NOLINT})
find_program(CLANG_TIDY_COMMAND clang-tidy)
if(CLANG_TIDY_COMMAND)

# Clang-tidy for linting
set(CMAKE_CXX_CLANG_TIDY 
  clang-tidy;
  -checks=google-*,cppcoreguidelines-*
)
set(CMAKE_C_CLANG_TIDY 
  clang-tidy;
  -checks=google-*,cppcoreguidelines-*
)

else()
message(WARNING "Unable to find clang-tidy, linter disabled")
endif()

else()
message(WARNING "-DNOLINT option provided, linter disabled")
endif()

set(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(BASH_PROGRAM bash)

# Add flags for GNU sampling profiler gprof in Debug build and RelWithDebInfo
option(PROFILE OFF)
if (PROFILE)
  add_compile_options(-pg)
  add_link_options(-pg)
endif()

option(SANITIZE OFF)
if (SANITIZE)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

add_subdirectory(spdlog EXCLUDE_FROM_ALL)
add_subdirectory(glm EXCLUDE_FROM_ALL)
add_subdirectory(hw3d)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_program(glslc NAMES glslc HINTS Vulkan::glslc)

function(add_spirv_shader TARGET_NAME OUTPUT_FILE INPUT_FILE)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
    COMMAND ${glslc} -c ${INPUT_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/${OUTPUT_FILE}
    MAIN_DEPENDENCY ${INPUT_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  add_custom_target(${TARGET_NAME} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shaders/${OUTPUT_FILE})
endfunction()

if (NOT VulkanFOUND)
  find_library(Vulkan_LIBRARY REQUIRED)
  if (Vulkan_LIBRARY)
    set (Vulkan_FOUND ON)
  endif()
endif()

set(VK_VALIDATION_LAYER OFF CACHE BOOL "Enables Vulkan validation layer")

set(EZVK_SOURCES
  src/surface.cc
  src/window.cc
  src/shaders.cc
  src/instance.cc
  src/device.cc
  src/queue_families.cc
  src/pipeline.cc
  src/swapchain.cc
  src/logging.cc
  src/memory.cc
  src/ezvk/window.cc
)

add_library(ezvk ${EZVK_SOURCES})
if (VK_VALIDATION_LAYER)
  message("Vulkan validation layer enabled")
  target_compile_definitions(ezvk PUBLIC VK_VALIDATION_LAYER)
endif()

target_include_directories(ezvk PUBLIC include)
target_link_libraries(ezvk INTERFACE spdlog_header_only)

set(APPLICATION_SOURCES
  app/main.cc
)

add_spirv_shader(vertex vertex.spv shaders/shader.vert)
add_spirv_shader(fragment fragment.spv shaders/shader.frag)
add_custom_target(shaders ALL DEPENDS vertex fragment)

add_executable(triangles ${APPLICATION_SOURCES})
target_link_libraries(triangles PUBLIC glfw Vulkan::Vulkan ezvk glm throttle)
add_dependencies(triangles shaders)

add_executable(ezvk_test app/ezvk_test.cc)
target_link_libraries(ezvk_test PUBLIC glfw Vulkan::Vulkan ezvk glm throttle)

install(TARGETS triangles DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)