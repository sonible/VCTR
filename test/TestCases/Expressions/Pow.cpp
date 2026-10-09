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

TEMPLATE_PRODUCT_TEST_CASE ("Pow", "[VCTR][Expressions][pow]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, int64_t, std::complex<float>, std::complex<double>))
{
    VCTR_TEST_DEFINES_IN_RANGE (0, 15, 10)

    // For non-integral values we want to test with negative exponents
    constexpr int singleExponent = std::is_integral_v<ElementType> ? 5 : -4;
    constexpr int singleBase = 3;
    constexpr int constantBase = 4;

    // Plain callables instead of overloaded function templates used as non-type template arguments
    const auto power = [] (ElementType base, ElementType exp) { return std::pow (base, exp); };
    const auto powerConstantExp = [] (ElementType base) { return std::pow (base, ElementType (singleExponent)); };
    const auto powerSingleBase = [] (ElementType exp) { return std::pow (ElementType (singleBase), exp); };
    const auto powerConstantBase = [] (ElementType exp) { return std::pow (ElementType (constantBase), exp); };

    SECTION ("Vector raised to the power of Vector")
    {
        const vctr::Vector p = filter << vctr::pow (srcA, srcB);
        REQUIRE_THAT (p, vctr::EqualsMappedBy (power, srcA, srcB).withEpsilon (0.00001));
    }

    SECTION ("Vector raised to the power of a single value")
    {
        const vctr::Vector p = filter << vctr::pow (srcA, ElementType (singleExponent));
        REQUIRE_THAT (p, vctr::EqualsMappedBy (powerConstantExp, srcA).withEpsilon (0.00001));
    }

    SECTION ("Single value raised to the power of Vector")
    {
        const vctr::Vector p = filter << vctr::pow (ElementType (singleBase), srcA);
        REQUIRE_THAT (p, vctr::EqualsMappedBy (powerSingleBase, srcA));
    }

    SECTION ("Vector raised to the power of a compile time constant value")
    {
        const vctr::Vector p = filter << vctr::powConstantExponent<singleExponent> << srcA;
        REQUIRE_THAT (p, vctr::EqualsMappedBy (powerConstantExp, srcA).withEpsilon (0.00001));
    }

    SECTION ("Compile time constant value raised to the power of Vector")
    {
        const vctr::Vector p = filter << vctr::powConstantBase<constantBase> << srcA;
        REQUIRE_THAT (p, vctr::EqualsMappedBy (powerConstantBase, srcA));
    }
}

