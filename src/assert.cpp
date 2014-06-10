#include "pch.hpp"
#include "assert.hpp"

#include <iostream>

void yama::detail::assertion_handler(
    char const* condition
  , char const* file
  , int line
  , char const* function
) {
    std::cout << condition << file << line << function << std::endl;
}
