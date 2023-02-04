/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy us a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <concepts>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace ezvk::utils {

inline std::vector<char> read_file(std::string filename) {
  std::ifstream file;

  file.exceptions(file.exceptions() | std::ifstream::failbit | std::ifstream::badbit);
  file.open(filename, std::ios::binary);

  std::istreambuf_iterator<char> start(file), fin;
  return std::vector<char>(start, fin);
}

auto find_all_missing(auto all_start, auto all_finish, auto find_start, auto find_finish, auto proj) {
  std::vector<typename decltype(find_start)::value_type> missing;

  for (; find_start != find_finish; ++find_start) {
    if (std::find_if(all_start, all_finish, [find_start, proj](auto a) { return proj(a) == *find_start; }) !=
        all_finish)
      continue;
    missing.push_back(*find_start);
  }

  return missing;
}

auto find_all_that_satisfy(auto all_start, auto all_finish, auto pred) {
  std::vector<typename std::iterator_traits<decltype(all_start)>::value_type> satisfy;

  for (; all_start != all_finish; ++all_start) {
    if (pred(*all_start)) satisfy.push_back(*all_start);
  }

  return satisfy;
}

inline std::string trim_leading_trailing_spaces(std::string input) {
  if (input.empty()) return {};
  auto pos_first = input.find_first_not_of(" \t\n");
  auto pos_last = input.find_last_not_of(" \t\n");
  return input.substr(pos_first != std::string::npos ? pos_first : 0, (pos_last - pos_first) + 1);
}

template <typename T> auto sizeof_container(const T &container) {
  return sizeof(typename T::value_type) * container.size();
}

}; // namespace ezvk::utils