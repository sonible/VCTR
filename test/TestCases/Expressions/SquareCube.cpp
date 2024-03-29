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

#include <vctr_test_utils/vctr_test_common.h>

template <vctr::is::realFloatNumber T>
T sqrt (T x) { return std::sqrt (x); }

template <vctr::is::complexFloatNumber T>
T sqrt (T x) { return std::sqrt (x); }

template <class T>
T square (T x) { return x * x; }

template <class T>
T cube (T x) { return x * x * x; }

TEMPLATE_PRODUCT_TEST_CASE ("Square Root", "[sqrt]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_IN_RANGE (0, 10000, 10)

    const vctr::Vector s = filter << vctr::sqrt << srcA;

    REQUIRE_THAT (s, vctr::EqualsTransformedBy<sqrt> (srcA).withEpsilon (0.000001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Square", "[square]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_IN_RANGE (-20, 20, 10)

    const vctr::Vector s = filter << vctr::square << srcA;

    REQUIRE_THAT (s, vctr::EqualsTransformedBy<square> (srcA));
}

TEMPLATE_PRODUCT_TEST_CASE ("Cube", "[cube]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector s = filter << vctr::cube << srcA;

    REQUIRE_THAT (s, vctr::EqualsTransformedBy<cube> (srcA).withEpsilon (0.000001));
}
