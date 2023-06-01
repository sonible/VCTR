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
T sin (T v) { return std::sin (v); }

template <vctr::is::realOrComplexFloatNumber T>
T cos (T v) { return std::cos (v); }

template <vctr::is::realOrComplexFloatNumber T>
T tan (T v) { return std::tan (v); }

template <vctr::is::realOrComplexFloatNumber T>
T sinh (T v) { return std::sinh (v); }

template <vctr::is::realOrComplexFloatNumber T>
T cosh (T v) { return std::cosh (v); }

template <vctr::is::realOrComplexFloatNumber T>
T tanh (T v) { return std::tanh (v); }

template <vctr::is::realOrComplexFloatNumber T>
T asin (T v) { return std::asin (v); }

template <vctr::is::realOrComplexFloatNumber T>
T acos (T v) { return std::acos (v); }

template <vctr::is::realOrComplexFloatNumber T>
T atan (T v) { return std::atan (v); }

template <vctr::is::realOrComplexFloatNumber T>
T asinh (T v) { return std::asinh (v); }

template <vctr::is::realOrComplexFloatNumber T>
T acosh (T v) { return std::acosh (v); }

template <vctr::is::realOrComplexFloatNumber T>
T atanh (T v) { return std::atanh (v); }


TEMPLATE_PRODUCT_TEST_CASE ("Sin", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::sin << srcA;
    const vctr::Vector resU = filter << vctr::sin << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<sin> (srcA).withMargin (0.0001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<sin> (srcUnaligned).withMargin (0.0001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Cos", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::cos << srcA;
    const vctr::Vector resU = filter << vctr::cos << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<cos> (srcA).withMargin (0.0001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<cos> (srcUnaligned).withMargin (0.0001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Tan", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::tan << srcA;
    const vctr::Vector resU = filter << vctr::tan << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<tan> (srcA).withMargin (0.0001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<tan> (srcUnaligned).withMargin (0.0001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Sinh", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::sinh << srcA;
    const vctr::Vector resU = filter << vctr::sinh << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<sinh> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<sinh> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Cosh", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::cosh << srcA;
    const vctr::Vector resU = filter << vctr::cosh << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<cosh> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<cosh> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Tanh", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::tanh << srcA;
    const vctr::Vector resU = filter << vctr::tanh << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<tanh> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<tanh> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Asin", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-1, 1, 10)

    const vctr::Vector res = filter << vctr::asin << srcA;
    const vctr::Vector resU = filter << vctr::asin << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<asin> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<asin> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Acos", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-1, 1, 10)

    const vctr::Vector res = filter << vctr::acos << srcA;
    const vctr::Vector resU = filter << vctr::acos << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<acos> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<acos> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Atan", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::atan << srcA;
    const vctr::Vector resU = filter << vctr::atan << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<atan> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<atan> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Asinh", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-4, 4, 10)

    const vctr::Vector res = filter << vctr::asinh << srcA;
    const vctr::Vector resU = filter << vctr::asinh << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<asinh> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<asinh> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Acosh", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (1, 10, 10)

    const vctr::Vector res = filter << vctr::acosh << srcA;
    const vctr::Vector resU = filter << vctr::acosh << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<acosh> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<acosh> (srcUnaligned).withMargin (0.00001));
}

TEMPLATE_PRODUCT_TEST_CASE ("Atanh", "[expressions][trigonometric]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (-1, 1, 10)

    const vctr::Vector res = filter << vctr::atanh << srcA;
    const vctr::Vector resU = filter << vctr::atanh << srcUnaligned;

    REQUIRE_THAT (res, vctr::EqualsTransformedBy<atanh> (srcA).withMargin (0.00001));
    REQUIRE_THAT (resU, vctr::EqualsTransformedBy<atanh> (srcUnaligned).withMargin (0.00001));
}