set(LIBRARY_SOURCES
  src/temp.cc
)

add_library(throttle ${LIBRARY_SOURCES})
target_include_directories(throttle PUBLIC include)
target_link_libraries(throttle mpark_variant)

set(UNIT_TEST_SOURCES
  test/main.cc
  test/test_equal.cc
  test/test_vec3.cc
  test/test_vec2.cc
  test/test_point2.cc
  test/test_point3.cc
  test/test_plane.cc
  test/test_triangle3.cc
  test/test_aabb.cc
  test/test_line2.cc
  test/test_segment1.cc
  test/test_segment2.cc
  test/test_segment3.cc
  test/test_triangle2.cc
)

if (ENABLE_GTEST)
  add_executable(unit_test ${UNIT_TEST_SOURCES})
  target_include_directories(unit_test PRIVATE src include)
  target_link_libraries(unit_test throttle ${GTEST_BOTH_LIBRARIES})
  gtest_discover_tests(unit_test)
endif()