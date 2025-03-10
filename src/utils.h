#include <spdlog/logger.h>

#include <cstddef>
#include <format>
#include <sstream>

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

#ifndef INCLUDE_SRC_UTILS_H_
#define INCLUDE_SRC_UTILS_H_

template <typename T>
std::string _from_container(typename T::iterator beg, typename T::iterator end,
                            size_t sz) {
  std::ostringstream ss;
  ss << std::format("[{}] {{", sz);
  if (beg != end) {
    for (; next(beg) != end; ++beg) {
      ss << *beg << ",";
    }
    ss << *beg;
  }
  ss << "}";
  return ss.str();
};

#endif  // INCLUDE_SRC_UTILS_H_
