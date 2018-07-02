//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <sstream>
#include <string>

namespace http {
namespace server {

struct header
{
  std::string name;
  std::string value;
};

template <typename T>
inline std::string InternalToString(T value) {
#ifndef OS_ANDROID
  return std::to_string(value);
#else
  // Andorid doesn't support all of C++11, std::to_string() being
  // one of the not supported features.
  std::ostringstream os;
  os << value;
  return os.str();
#endif
}


} // namespace server
} // namespace http

#endif // HTTP_HEADER_HPP
