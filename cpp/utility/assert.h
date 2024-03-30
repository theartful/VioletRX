#ifndef VIOLET_ASSERT_H
#define VIOLET_ASSERT_H

#include <stdexcept>

#include <fmt/format.h>

#define VIOLET_ASSERT(expr)                                                    \
    if (!static_cast<bool>(expr)) {                                            \
        ::asserts::assert_fail(#expr, __FILE__, __LINE__, __func__);           \
    }

namespace asserts
{
[[noreturn]] static inline void assert_fail(const char* assertion,
                                            const char* file, unsigned int line,
                                            const char* function)
{

    throw std::logic_error(fmt::format("[{}:{} {}] Assertion failed: %s", file,
                                       line, function, assertion));
}

} // namespace asserts

#endif
