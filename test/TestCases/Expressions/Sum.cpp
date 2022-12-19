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

TEMPLATE_PRODUCT_TEST_CASE ("Sum", "[sum]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>, int32_t, int64_t, uint32_t, uint64_t) )
{
    VCTR_TEST_DEFINES (10)

    const auto sum = vctr::sum << filter << srcA;
    const auto sumU = vctr::sum << filter << srcUnaligned;

    const auto sumMemFn = srcA.sum();
    const auto ref = std::reduce (srcA.begin(), srcA.end());
    const auto refU = std::reduce (srcUnaligned.begin(), srcUnaligned.end());

    const auto eps = vctr::RealType<ElementType> (0.00001);

    if constexpr (vctr::is::complexFloatNumber<ElementType>)
    {
        REQUIRE_THAT (sum.real(), Catch::Matchers::WithinRel (sumMemFn.real(), eps));
        REQUIRE_THAT (sum.imag(), Catch::Matchers::WithinRel (sumMemFn.imag(), eps));
        REQUIRE_THAT (sum.real(), Catch::Matchers::WithinRel (ref.real(), eps));
        REQUIRE_THAT (sum.imag(), Catch::Matchers::WithinRel (ref.imag(), eps));
        REQUIRE_THAT (sumU.real(), Catch::Matchers::WithinRel (refU.real(), eps));
        REQUIRE_THAT (sumU.imag(), Catch::Matchers::WithinRel (refU.imag(), eps));
    }
    else if constexpr (vctr::is::floatNumber<ElementType>)
    {
        REQUIRE_THAT (sum, Catch::Matchers::WithinRel (sumMemFn, eps));
        REQUIRE_THAT (sum, Catch::Matchers::WithinRel (ref, eps));
        REQUIRE_THAT (sumU, Catch::Matchers::WithinRel (refU, eps));
    }
    else
    {
        REQUIRE (sum == sumMemFn);
        REQUIRE (sum == ref);
        REQUIRE (sumU == refU);
    }
}

TEST_CASE ("Sum strings", "[sum]")
{
    const auto loremIpsum = UnitTestValues<std::string>::template array<10, 0>();

    const auto sum = vctr::sum << loremIpsum;

    REQUIRE (sum == std::reduce (loremIpsum.begin(), loremIpsum.end()));
}