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

template <vctr::is::realFloatNumber T>
T stdExp2 (T v) { return std::exp2 (v); }

template <vctr::is::realFloatNumber T>
T stdLog2 (T v) { return std::log2 (v); }

template <vctr::is::realOrComplexFloatNumber T>
T fastExp (T v) { return (T (1680) + v * (T (840) + v * (T (180) + v * (T (20) + v)))) / (T (1680) + v * (T (-840) + v * (T (180) + v * (T (-20) + v)))); }

TEMPLATE_PRODUCT_TEST_CASE ("FastExp vs. FastExp", "[VCTR][Expressions][fastExp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_IN_RANGE (-6, 4, 10)

    const vctr::Vector res = filter << vctr::fastExp << srcA;
    const vctr::Vector resU = filter << vctr::fastExp << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<fastExp> (srcA).withEpsilon (1e-4));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<fastExp> (srcUnaligned).withEpsilon (1e-4));
}

TEMPLATE_PRODUCT_TEST_CASE ("FastExp vs. StdExp", "[VCTR][Expressions][fastExp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES_IN_RANGE (-2, 2, 10)

    const vctr::Vector res = filter << vctr::fastExp << srcA;
    const vctr::Vector resU = filter << vctr::fastExp << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<stdExp> (srcA).withMargin (0.01));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<stdExp> (srcUnaligned).withMargin (0.01));
}

TEMPLATE_PRODUCT_TEST_CASE ("FastExp2 vs. StdExp2", "[VCTR][Expressions][fastExp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double) )
{
    constexpr auto startValue = int (vctr::expressions::detail::FastExp2Constants<typename TestType::ElementType>::minExpo);
    constexpr auto endValue = int (vctr::expressions::detail::FastExp2Constants<typename TestType::ElementType>::expoBias) + 1;

    VCTR_TEST_DEFINES_IN_RANGE (startValue, endValue, 10)

    const vctr::Vector res = filter << vctr::fastExp2 << srcA;
    const vctr::Vector resU = filter << vctr::fastExp2 << srcUnaligned;

    CHECK_THAT (res, vctr::EqualsTransformedBy<stdExp2> (srcA).withEpsilon (1e-4));
    CHECK_THAT (resU, vctr::EqualsTransformedBy<stdExp2> (srcUnaligned).withEpsilon (1e-4));
}

TEMPLATE_PRODUCT_TEST_CASE ("FastLog2 vs. StdLog2", "[VCTR][Expressions][fastExp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float) )
{
    VCTR_TEST_DEFINES_IN_RANGE (0, 126, 10)

    const vctr::Vector res = filter << vctr::fastLog2 << srcA;
    const vctr::Vector resU = filter << vctr::fastLog2 << srcUnaligned;

    CHECK_THAT (res, vctr::EqualsTransformedBy<stdLog2> (srcA).withEpsilon (2.0e-2));
    CHECK_THAT (resU, vctr::EqualsTransformedBy<stdLog2> (srcUnaligned).withEpsilon (2.0e-2));
}