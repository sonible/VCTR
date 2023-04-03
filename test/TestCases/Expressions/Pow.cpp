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

// The Linux builds failed with templated functions. Could be cleaned up at some point.
// clang-format off
float                power (float base,                float exp)                { return std::pow (base, exp); }
double               power (double base,               double exp)               { return std::pow (base, exp); }
double               power (int32_t base,              int32_t exp)              { return std::pow (base, exp); }
double               power (int64_t base,              int64_t exp)              { return std::pow (base, exp); }
std::complex<float>  power (std::complex<float> base,  std::complex<float> exp)  { return std::pow (base, exp); }
std::complex<double> power (std::complex<double> base, std::complex<double> exp) { return std::pow (base, exp); }

template <int32_t base> float                powerConstantBase (float exp)                { return std::pow (float (base), exp); }
template <int32_t base> double               powerConstantBase (double exp)               { return std::pow (double (base), exp); }
template <int32_t base> double               powerConstantBase (int32_t exp)              { return std::pow (base, exp); }
template <int64_t base> double               powerConstantBase (int64_t exp)              { return std::pow (base, exp); }
template <int32_t base> std::complex<float>  powerConstantBase (std::complex<float> exp)  { return std::pow (std::complex<float> (base), exp); }
template <int32_t base> std::complex<double> powerConstantBase (std::complex<double> exp) { return std::pow (std::complex<double> (base), exp); }

template <int32_t exp> float                powerConstantExp (float base)                { return std::pow (base, exp); }
template <int32_t exp> double               powerConstantExp (double base)               { return std::pow (base, exp); }
template <int32_t exp> double               powerConstantExp (int32_t base)              { return std::pow (base, exp); }
template <int64_t exp> double               powerConstantExp (int64_t base)              { return std::pow (base, exp); }
template <int32_t exp> std::complex<float>  powerConstantExp (std::complex<float> base)  { return std::pow (base, std::complex<float> (exp)); }
template <int32_t exp> std::complex<double> powerConstantExp (std::complex<double> base) { return std::pow (base, std::complex<double> (exp)); }
// clang-format on

TEMPLATE_PRODUCT_TEST_CASE ("Pow", "[pow]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, int64_t, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (0, 15, 10)

    // For non-integral values we want to test with negative exponents
    constexpr int singleExponent = std::is_integral_v<ElementType> ? 5 : -4;

    SECTION ("Vector raised to the power of Vector")
    {
        const vctr::Vector p = filter << vctr::pow (srcA, srcB);
        REQUIRE_THAT (p, vctr::EqualsTransformedBy<power> (srcA, srcB).withEpsilon (0.00001));
    }

    SECTION ("Vector raised to the power of a single value")
    {
        const vctr::Vector p = filter << vctr::pow (srcA, ElementType (singleExponent));
        REQUIRE_THAT (p, vctr::EqualsTransformedBy<powerConstantExp<singleExponent>> (srcA).withEpsilon (0.00001));
    }

    SECTION ("Single value raised to the power of Vector")
    {
        const vctr::Vector p = filter << vctr::pow (3.0f, srcA);
        REQUIRE_THAT (p, vctr::EqualsTransformedBy<powerConstantBase<3>> (srcA));
    }

    SECTION ("Vector raised to the power of a compile time constant value")
    {
        const vctr::Vector p = filter << vctr::powConstantExponent<singleExponent> << srcA;
        REQUIRE_THAT (p, vctr::EqualsTransformedBy<powerConstantExp<singleExponent>> (srcA).withEpsilon (0.00001));
    }

    SECTION ("Compile time constant value raised to the power of Vector")
    {
        const vctr::Vector p = filter << vctr::powConstantBase<4> << srcA;
        REQUIRE_THAT (p, vctr::EqualsTransformedBy<powerConstantBase<4>> (srcA));
    }
}