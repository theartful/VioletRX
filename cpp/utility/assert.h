#ifndef VIOLET_ASSERT_H
#define VIOLET_ASSERT_H

#define VIOLET_ASSERT(expr)                                                    \
    if (!static_cast<bool>(expr)) {                                            \
        ::asserts::assert_fail(#expr, __FILE__, __LINE__, __func__);           \
    }

namespace asserts
{
[[noreturn]] void assert_fail(const char* assertion, const char* file,
                              unsigned int line, const char* function);
}

#endif
