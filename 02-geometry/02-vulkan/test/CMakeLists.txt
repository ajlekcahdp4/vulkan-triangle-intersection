set (APPLICATION_SOURCES
    src/main.cc
)

add_executable(application ${APPLICATION_SOURCES})
target_include_directories(application PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(application PRIVATE ${ENGINE_INCLUDE_DIR})
target_link_libraries(application PRIVATE glfw Vulkan::Vulkan)
install (TARGETS application DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)