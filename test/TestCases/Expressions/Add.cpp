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
T addition (T a, T b) { return a + b; }

TEMPLATE_PRODUCT_TEST_CASE ("Addition", "[add]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const auto c = srcC[0];

    const vctr::Vector sum1 = filter << (c + srcA);
    const vctr::Vector sum2 = filter << (srcA + c);
    const vctr::Vector sum3 = filter << (srcA + srcB);

    REQUIRE_THAT (sum1, vctr::EqualsTransformedBy<addition> (srcA, c));
    REQUIRE_THAT (sum2, vctr::EqualsTransformedBy<addition> (srcA, c));
    REQUIRE_THAT (sum3, vctr::EqualsTransformedBy<addition> (srcA, srcB));
}

TEMPLATE_PRODUCT_TEST_CASE ("Addition in place", "[add]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const auto c = srcC[0];

    auto sum1 = srcA;
    auto sum2 = srcA;

    sum1 += srcB;
    sum2 += c;

    REQUIRE_THAT (sum1, vctr::EqualsTransformedBy<addition> (srcA, srcB).withEpsilon());
    REQUIRE_THAT (sum2, vctr::EqualsTransformedBy<addition> (srcA, c).withEpsilon());

    // Operator += implements special accelerated handling for the case of multiply-accumulate operations
    auto mac1 = srcA;
    auto mac2 = srcA;
    //auto mac3 = srcA;

    mac1 += srcB * srcC;
    mac2 += srcB * c;
    // Todo: This causes build errors on some platforms. https://github.com/sonible/VCTR/issues/70
    //mac3 += vctr::multiplyByConstant<42> << srcB;

    const auto eps = vctr::RealType<ElementType> (0.00001);

    for (size_t i = 0; i < 10; ++i)
    {
        if constexpr (vctr::is::complexFloatNumber<ElementType>)
        {
            REQUIRE_THAT (mac1[i].real(), Catch::Matchers::WithinRel ((srcA[i] + srcB[i] * srcC[i]).real(), eps));
            REQUIRE_THAT (mac1[i].imag(), Catch::Matchers::WithinRel ((srcA[i] + srcB[i] * srcC[i]).imag(), eps));
            REQUIRE_THAT (mac2[i].real(), Catch::Matchers::WithinRel ((srcA[i] + srcB[i] * c).real(), eps));
            REQUIRE_THAT (mac2[i].imag(), Catch::Matchers::WithinRel ((srcA[i] + srcB[i] * c).imag(), eps));
        }
        else if constexpr (vctr::is::realFloatNumber<ElementType>)
        {
            REQUIRE_THAT (mac1[i], Catch::Matchers::WithinRel (srcA[i] + srcB[i] * srcC[i], eps));
            REQUIRE_THAT (mac2[i], Catch::Matchers::WithinRel (srcA[i] + srcB[i] * c, eps));
        }
        else
        {
            REQUIRE (mac1[i] == srcA[i] + srcB[i] * srcC[i]);
            REQUIRE (mac2[i] == srcA[i] + srcB[i] * c);
        }
    }
}
