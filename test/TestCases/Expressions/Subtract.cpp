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

template <class T>
T subtraction (T a, T b) { return a - b; }

TEMPLATE_PRODUCT_TEST_CASE ("Subtraction", "[subtract]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const auto c = srcC[0];

    const vctr::Vector constantMinusVec = filter << (c - srcA);
    const vctr::Vector vecMinusConstant = filter << (srcA - c);
    const vctr::Vector vecMinusVec = filter << (srcA - srcB);

    REQUIRE_THAT (constantMinusVec, vctr::EqualsTransformedBy<subtraction> (c, srcA));
    REQUIRE_THAT (vecMinusConstant, vctr::EqualsTransformedBy<subtraction> (srcA, c));
    REQUIRE_THAT (vecMinusVec, vctr::EqualsTransformedBy<subtraction> (srcA, srcB));
}

TEMPLATE_PRODUCT_TEST_CASE ("Subtraction in place", "[subtract]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>, std::complex<int32_t>, std::complex<int64_t>) )
{
    VCTR_TEST_DEFINES (10)

    const auto c = srcC[0];

    auto diff1 = srcA;
    auto diff2 = srcA;

    diff1 -= srcB;
    diff2 -= c;

    REQUIRE_THAT (diff1, vctr::EqualsTransformedBy<subtraction> (srcA, srcB).withEpsilon());
    REQUIRE_THAT (diff2, vctr::EqualsTransformedBy<subtraction> (srcA, c).withEpsilon());
}
