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
