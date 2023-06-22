/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2022- by sonible GmbH.

    This file is part of VCTR - Versatile Container Templates Reconceptualized.

    VCTR is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    VCTR is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License version 3 for more details.

    You should have received a copy of the GNU Lesser General Public License
    version 3 along with VCTR.  If not, see <https://www.gnu.org/licenses/>.
  ==============================================================================
*/

#define VCTR_TO_STRING(s) #s

#if VCTR_MSVC
#define VCTR_TARGET(arch)
#else
#define VCTR_TARGET(arch) __attribute__ ((target (arch)))
#endif

// VCTR_FORCEDINLINE enforces full inlining in release builds. In debug builds we want to preserve the possibility
// to debug, so it does not affect inlining there
#if VCTR_DEBUG
#define VCTR_FORCEDINLINE
#else
#if VCTR_MSVC
#define VCTR_FORCEDINLINE __forceinline
#else
#define VCTR_FORCEDINLINE inline __attribute__ ((always_inline))
#endif
#endif


#if (VCTR_GCC || VCTR_CLANG)

#define VCTR_JOIN_DIAGNOSTICS_STRINGS(x,y) VCTR_TO_STRING (x ## y)
#define VCTR_DIAGNOSTIC_PRAGMA(compiler, x) _Pragma (VCTR_TO_STRING (compiler diagnostic x))

#define VCTR_START_IGNORE_WARNING_CLANG_GCC(warning)  \
    VCTR_DIAGNOSTIC_PRAGMA (VCTR_COMPILER_NAME, push) \
    VCTR_DIAGNOSTIC_PRAGMA (VCTR_COMPILER_NAME, ignored VCTR_JOIN_DIAGNOSTICS_STRINGS (-W, warning))
#define VCTR_END_IGNORE_WARNING_CLANG_GCC \
    VCTR_DIAGNOSTIC_PRAGMA (VCTR_COMPILER_NAME, pop)

#else
#define VCTR_START_IGNORE_WARNING_CLANG_GCC(warning)
#define VCTR_END_IGNORE_WARNING_CLANG_GCC
#endif

#if VCTR_GCC
#define VCTR_START_IGNORE_WARNING_GCC(warning) VCTR_START_IGNORE_WARNING_CLANG_GCC(warning)
#define VCTR_END_IGNORE_WARNING_GCC VCTR_END_IGNORE_WARNING_CLANG_GCC
#else
#define VCTR_START_IGNORE_WARNING_GCC(warning)
#define VCTR_END_IGNORE_WARNING_GCC
#endif

#if VCTR_CLANG
#define VCTR_START_IGNORE_WARNING_CLANG(warning) VCTR_START_IGNORE_WARNING_CLANG_GCC(warning)
#define VCTR_END_IGNORE_WARNING_CLANG VCTR_END_IGNORE_WARNING_CLANG_GCC
#else
#define VCTR_START_IGNORE_WARNING_CLANG(warning)
#define VCTR_END_IGNORE_WARNING_CLANG
#endif

#if VCTR_MSVC
#define VCTR_START_IGNORE_WARNING_MSVC(warningNumber) __pragma(warning(push)) __pragma(warning(disable:warningNumber))
#define VCTR_END_IGNORE_WARNING_MSVC __pragma(warning(pop))
#else
#define VCTR_START_IGNORE_WARNING_MSVC(warning)
#define VCTR_END_IGNORE_WARNING_MSVC
#endif
