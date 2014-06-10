#pragma once

namespace yama {

#define BK_DEBUG_BREAK __debugbreak

#define BK_ABORT_TODO []() -> void { BK_DEBUG_BREAK(); abort(); }

namespace detail {

void assertion_handler(char const* condition, char const* file, int line, char const* function);

} //namespace yama::detail

#define BK_ASSERT_IMPL(condition, file, line, function) \
[&]() -> void { \
    while (!(condition)) { \
        ::yama::detail::assertion_handler(#condition, file, line, function); \
        BK_DEBUG_BREAK(); \
    } \
}()

#define BK_ASSERT(condition) BK_ASSERT_IMPL(condition, __FILE__, __LINE__, __func__)

} //namespace yama
