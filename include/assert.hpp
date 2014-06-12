#pragma once

#include <boost/predef.h>

namespace yama {

#if BOOST_COMP_GNUC
inline void debug_break_function() { asm("int $3"); }
#define BK_DEBUG_BREAK debug_break_function
#elif BOOST_COMP_MSVC
#define BK_DEBUG_BREAK __debugbreak
#endif

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

#if !defined(NDEBUG)
#define BK_ASSERT(condition) BK_ASSERT_IMPL(condition, __FILE__, __LINE__, __func__)
#else
#define BK_ASSERT(condition) (void)0
#endif

} //namespace yama
