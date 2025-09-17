/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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

template <vctr::is::realOrComplexFloatNumber T>
T stdExp (T v) { return std::exp (v); }

template <vctr::is::realOrComplexFloatNumber T>
T fastExp (T v) { return (T (1680) + v * (T (840) + v * (T (180) + v * (T (20) + v)))) / (T (1680) + v * (T (-840) + v * (T (180) + v * (T (-20) + v)))); }

TEMPLATE_PRODUCT_TEST_CASE ("FastExp vs. FastExp", "[VCTR][expressions][fastExp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_IN_RANGE (-6, 4, 10)

    const vctr::Vector res = filter << vctr::fastExp << srcA;
    const vctr::Vector resU = filter << vctr::fastExp << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<fastExp> (srcA).withMargin (0.0001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<fastExp> (srcUnaligned).withMargin (0.0001));
}


TEMPLATE_PRODUCT_TEST_CASE ("FastExp vs. StdExp", "[VCTR][expressions][fastExp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_IN_RANGE (-2, 2, 10)

    const vctr::Vector res = filter << vctr::fastExp << srcA;
    const vctr::Vector resU = filter << vctr::fastExp << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<stdExp> (srcA).withMargin (0.01));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<stdExp> (srcUnaligned).withMargin (0.01));
}