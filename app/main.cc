/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "ezvk/debug.hpp"
#include "ezvk/device.hpp"
#include "ezvk/error.hpp"
#include "ezvk/queues.hpp"
#include "ezvk/window.hpp"
#include "glfw_include.hpp"
#include "vertex.hpp"
#include "vulkan_hpp_include.hpp"

#include "spdlog/cfg/env.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iterator>
#include <spdlog/spdlog.h>

#include "broadphase/broadphase_structure.hpp"
#include "broadphase/bruteforce.hpp"
#include "broadphase/octree.hpp"
#include "broadphase/uniform_grid.hpp"

#include "narrowphase/collision_shape.hpp"
#include "primitives/plane.hpp"
#include "primitives/triangle3.hpp"
#include "vec3.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "application.hpp"

#ifdef BOOST_FOUND__
#include <boost/program_options.hpp>
#include <boost/program_options/option.hpp>
namespace po = boost::program_options;
#endif

struct indexed_geom : public throttle::geometry::collision_shape<float> {
  unsigned index;
  indexed_geom(unsigned idx, auto &&base) : collision_shape{base}, index{idx} {};
};

using throttle::geometry::collision_shape;
using throttle::geometry::is_roughly_equal;
using throttle::geometry::point3;
using throttle::geometry::segment3;
using throttle::geometry::triangle3;
using throttle::geometry::vec3;

template <typename T>
throttle::geometry::collision_shape<T> shape_from_three_points(const point3<T> &a, const point3<T> &b,
                                                               const point3<T> &c) {
  auto ab = b - a, ac = c - a;

  if (throttle::geometry::colinear(ab, ac)) { // Either a segment or a point
    if (is_roughly_equal(ab, vec3<T>::zero()) && is_roughly_equal(ac, vec3<T>::zero())) {
      return throttle::geometry::barycentric_average<T>(a, b, c);
    }
    // This is a segment. Project the the points onto the most closely alligned axis.
    auto max_index = ab.max_component().first;

    std::array<std::pair<point3<T>, T>, 3> arr = {std::make_pair(a, a[max_index]), std::make_pair(b, b[max_index]),
                                                  std::make_pair(c, c[max_index])};
    std::sort(arr.begin(), arr.end(),
              [](const auto &left, const auto &right) -> bool { return left.second < right.second; });
    return segment3<T>{arr[0].first, arr[2].first};
  }

  return triangle3<T>{a, b, c};
}

static unsigned apporoximate_optimal_depth(unsigned number) {
  constexpr unsigned max_depth = 6;
  unsigned           log_num = std::log10(float(number));
  return std::min(max_depth, log_num);
}

template <typename broad>
bool application_loop(throttle::geometry::broadphase_structure<broad, indexed_geom> &cont,
                      std::vector<triangles::triangle_vertex_type> &vertices, unsigned n, bool hide = false) {
  using point_type = typename throttle::geometry::point3<float>;
  std::vector<point_type> points;
  points.reserve(n);
  for (unsigned i = 0; i < n; ++i) {
    point_type a, b, c;
    if (!(std::cin >> a[0] >> a[1] >> a[2] >> b[0] >> b[1] >> b[2] >> c[0] >> c[1] >> c[2])) {
      std::cout << "Can't read i-th = " << i << " triangle\n";
      return false;
    }
    cont.add_collision_shape({i, shape_from_three_points(a, b, c)});
    points.push_back(a);
    points.push_back(b);
    points.push_back(c);
  }

  auto result = cont.many_to_many();
  if (hide) return true;

  unsigned point_ind = 0;

  constexpr uint32_t intersect_index = 1u, regular_index = 0u;

  std::transform(points.begin(), points.end(), std::back_inserter(vertices), [&result, &point_ind](auto &point) {
    triangles::triangle_vertex_type vertex;
    vertex.pos = {point[0], point[1], point[2]};

    auto found =
        std::find_if(result.begin(), result.end(), [point_ind](auto shape) { return shape->index == point_ind / 3; });
    point_ind += 1;

    vertex.color_index = (found == result.end() ? regular_index : intersect_index);

    return vertex;
  });

  // Here we add triangles oriented in the opposite direction to light them differently
  for (unsigned i = 0; i < 3 * n; i += 3) {
    vertices.push_back(vertices[i]);
    vertices.push_back(vertices[i + 2]);
    vertices.push_back(vertices[i + 1]);
  }

  return true;
}

int main(int argc, char *argv[]) {
  // intersection

  bool hide = false;

#ifdef BOOST_FOUND__
  std::string             opt;
  po::options_description desc("Available options");
  desc.add_options()("help,h", "Print this help message")("measure,m", "Print perfomance metrics")(
      "hide", "Hide output")("broad", po::value<std::string>(&opt)->default_value("octree"),
                             "Algorithm for broad phase (bruteforce, octree, uniform-grid)");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  bool measure = vm.count("measure");
  hide = vm.count("hide");
#endif

  unsigned n;
  if (!(std::cin >> n)) {
    std::cout << "Can't read number of triangles\n";
    return 1;
  }

  std::vector<triangles::triangle_vertex_type> vertices;
  std::vector<triangles::triangle_vertex_type> wireframe_vertices{
      {{0.0f, 0.0f, 0.0f}, 0u}, {{1.0f, 1.0f, 1.0f}, 0u}, {{1.0f, 0.0f, 1.0f}, 0u}, {{0.0f, 1.0f, 0.0f}, 1u}};
  vertices.reserve(3 * n);

#ifdef BOOST_FOUND__
  auto start = std::chrono::high_resolution_clock::now();

  if (opt == "octree") {
    throttle::geometry::octree<float, indexed_geom> octree{apporoximate_optimal_depth(n)};
    if (!application_loop(octree, vertices, n, hide)) return 1;
  } else if (opt == "bruteforce") {
    throttle::geometry::bruteforce<float, indexed_geom> bruteforce{n};
    if (!application_loop(bruteforce, vertices, n, hide)) return 1;
  } else if (opt == "uniform-grid") {
    throttle::geometry::uniform_grid<float, indexed_geom> uniform{n};
    if (!application_loop(uniform, vertices, n, hide)) return 1;
  }

  auto finish = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration<double, std::milli>(finish - start);

  if (measure) {
    std::cout << opt << " took " << elapsed.count() << "ms to run\n";
  }

#else
  throttle::geometry::octree<float, indexed_geom> octree{apporoximate_optimal_depth(n)};
  application_loop(octree, vertices, n, hide);
#endif

  // visualisations

  ezvk::enable_glfw_exceptions();
  spdlog::cfg::load_env_levels(); // Read logging level from environment variables
  glfwInit();

  static constexpr auto app_info = vk::ApplicationInfo{.pApplicationName = "Hello, World!",
                                                       .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                                       .pEngineName = "Junk Inc.",
                                                       .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                                                       .apiVersion = VK_MAKE_VERSION(1, 1, 0)};

  vk::raii::Context ctx;

  auto extensions = triangles::required_vk_extensions();

#ifdef USE_DEBUG_EXTENSION
  auto layers = triangles::required_vk_layers(true);
#else
  auto                   layers = triangles::required_vk_layers();
#endif

  ezvk::instance raw_instance = {ctx, app_info, extensions.begin(), extensions.end(), layers.begin(), layers.end()};

#ifdef USE_DEBUG_EXTENSION
  ezvk::generic_instance instance = ezvk::debugged_instance{std::move(raw_instance)};
#else
  ezvk::generic_instance instance = std::move(raw_instance);
#endif

  auto physical_device_extensions = triangles::required_physical_device_extensions();
  auto suitable_physical_devices = ezvk::physical_device_selector::enumerate_suitable_physical_devices(
      instance(), physical_device_extensions.begin(), physical_device_extensions.end());

  if (suitable_physical_devices.empty()) {
    throw ezvk::vk_error{"No suitable physical devices found"};
  }

  auto p_device = std::move(suitable_physical_devices.front());
  auto window = ezvk::unique_glfw_window{"Triangles intersection", vk::Extent2D{800, 600}, true};
  auto surface = ezvk::surface{instance(), window};

#if 0
#endif

  triangles::applicaton_platform platform = {std::move(instance), std::move(window), std::move(surface),
                                             std::move(p_device)};

  auto &app = triangles::application::instance().get(&platform);

  app.load_triangles(vertices);
  app.load_wireframe(wireframe_vertices);

  while (!glfwWindowShouldClose(app.window())) {
    glfwPollEvents();
    app.loop();
  }

  app.shutdown();

  triangles::application::instance().destroy();
}