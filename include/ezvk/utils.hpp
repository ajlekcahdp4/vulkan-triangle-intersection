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
#include <functional>
#include <iostream>
#include <vector>

namespace ezvk::utils {

auto find_all_missing(auto all_start, auto all_finish, auto find_start, auto find_finish, auto proj)
    -> std::vector<typename decltype(find_start)::value_type> {
  std::vector<typename decltype(find_start)::value_type> missing;

  for (; find_start != find_finish; ++find_start) {
    if (std::find_if(all_start, all_finish, [find_start, proj](auto a) { return proj(a) == *find_start; }) !=
        all_finish)
      continue;
    missing.push_back(*find_start);
  }

  return missing;
}

}; // namespace ezvk::utils