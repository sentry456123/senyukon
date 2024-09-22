#pragma once

#if defined(_MSC_VER)
#define SEN_UNREACHABLE() __assume(0)
#elif defined(__GNUC__)
#define SEN_UNREACHABLE() __builtin_unreachable()
#else
#define SEN_UNREACHABLE()
#endif
