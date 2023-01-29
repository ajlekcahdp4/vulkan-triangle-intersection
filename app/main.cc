#include "application.hpp"
#include "spdlog/cfg/env.h"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

int main() {
  spdlog::cfg::load_env_levels();
  glfwInit();
  triangles::application app;
  app.run();
}