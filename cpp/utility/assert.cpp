#include "assert.h"

#include <fmt/format.h>
#include <stdexcept>

namespace asserts
{
void assert_fail(const char* assertion, const char* file, unsigned int line,
                 const char* function)
{
    throw std::logic_error(fmt::format("[{}:{} {}] Assertion failed: %s", file,
                                       line, function, assertion));
}

} // namespace asserts
