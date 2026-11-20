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
static void requireProductEquals (T product, T ref)
{
    const auto eps = vctr::RealType<T> (0.0001);

    if constexpr (vctr::is::complexFloatNumber<T>)
    {
        REQUIRE_THAT (product.real(), Catch::Matchers::WithinRel (ref.real(), eps));
        REQUIRE_THAT (product.imag(), Catch::Matchers::WithinRel (ref.imag(), eps));
    }
    else if constexpr (vctr::is::realFloatNumber<T>)
    {
        REQUIRE_THAT (product, Catch::Matchers::WithinRel (ref, eps));
    }
    else
    {
        REQUIRE (product == ref);
    }
}

template <class Src>
static auto referenceProduct (const Src& src)
{
    return std::accumulate (src.begin(), src.end(), vctr::ValueType<Src> (1), std::multiplies<>());
}

// Small values, so that the product of 10 elements neither overflows integers nor loses float precision
TEMPLATE_PRODUCT_TEST_CASE ("Product", "[VCTR][Expressions][product]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, std::complex<float>, std::complex<double>, int32_t, int64_t) )
{
    VCTR_TEST_DEFINES_NO_ZEROS_IN_RANGE (-3, 3, 10)

    const auto product = vctr::product << filter << srcA;
    const auto productU = vctr::product << filter << srcUnaligned;

    requireProductEquals (product, referenceProduct (srcA));
    requireProductEquals (productU, referenceProduct (srcUnaligned));

    static constexpr auto productConstexpr = vctr::product << filter << srcA;
    requireProductEquals (productConstexpr, referenceProduct (srcA));
}

TEMPLATE_PRODUCT_TEST_CASE ("Product unsigned", "[VCTR][Expressions][product]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (uint32_t, uint64_t) )
{
    VCTR_TEST_DEFINES_NO_ZEROS_IN_RANGE (1, 4, 10)

    const auto product = vctr::product << filter << srcA;
    const auto productU = vctr::product << filter << srcUnaligned;

    requireProductEquals (product, referenceProduct (srcA));
    requireProductEquals (productU, referenceProduct (srcUnaligned));
}

TEMPLATE_PRODUCT_TEST_CASE ("Product with zero", "[VCTR][Expressions][product]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t) )
{
    VCTR_TEST_DEFINES_WITH_TRAILING_ZERO_IN_RANGE (-3, 3, 10)

    REQUIRE ((vctr::product << filter << srcA) == ElementType (0));
}
