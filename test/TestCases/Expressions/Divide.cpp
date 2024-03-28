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

#if VCTR_MSVC
// MSVC emits warnings about instanciations of division<std::complex<uint32_t>> - which is probably a false warning
// generated during overload resolution for vctr::EqualsTransformedBy
#pragma warning(disable : 4146)
#endif

TEMPLATE_PRODUCT_TEST_CASE ("Division", "[divide]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_NO_ZEROS (10)

    auto division = [] (ElementType a, ElementType b) { return a / b; };

    const auto c = srcC[0];

    // clang-format off
    const vctr::Vector constantDividedByVec = filter << (c / srcA);
    const vctr::Vector vecDividedByConstant = filter << (srcA / c);
    const vctr::Vector vecDividedByVec      = filter << (srcA / srcB);

    REQUIRE_THAT (constantDividedByVec, vctr::EqualsTransformedBy<division> (c, srcA).withEpsilon());
    REQUIRE_THAT (vecDividedByConstant, vctr::EqualsTransformedBy<division> (srcA, c).withEpsilon());
    REQUIRE_THAT (vecDividedByVec,      vctr::EqualsTransformedBy<division> (srcA, srcB).withEpsilon());
    // clang-format on
}

TEMPLATE_PRODUCT_TEST_CASE ("Divide in place", "[divide]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_NO_ZEROS (10)

    auto division = [] (ElementType a, ElementType b) { return a / b; };

    const auto c = srcC[0];

    auto quotient1 = srcA;
    auto quotient2 = srcA;

    quotient1 /= srcB;
    quotient2 /= c;

    REQUIRE_THAT (quotient1, vctr::EqualsTransformedBy<division> (srcA, srcB).withEpsilon());
    REQUIRE_THAT (quotient2, vctr::EqualsTransformedBy<division> (srcA, c).withEpsilon());
}
