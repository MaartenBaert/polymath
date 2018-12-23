#pragma once

#include <cassert>
#include <cstdint>

#include <algorithm>

#ifndef WIDEMATH_USE_ASM
#define WIDEMATH_USE_ASM 1
#endif

#if defined(__GCC_ASM_FLAG_OUTPUTS__) && !defined(Q_CREATOR_RUN)
#define ASM_FLAG_SET(flag, reg)
#define ASM_FLAG_OUT(flag, var) "=@cc" flag (var)
#else
#define ASM_FLAG_SET(flag, reg) " \n\tset" flag " " reg
#define ASM_FLAG_OUT(flag, var) "=r" (var)
#endif
