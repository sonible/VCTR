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
// MSVC emits warnings about instanciations of div10exp<std::complex<uint32_t>> - which is probably a false warning
// generated during overload resolution for vctr::EqualsTransformedBy
#pragma warning(disable : 4146)
#endif

// We chain a division by 10 and an exp operation in the test to not feed the exp operation with too big values.
// This function recreates that for the matcher
template <vctr::is::realNumber T>
auto div10exp (T x) { return std::exp (x / T (10)); }

TEMPLATE_PRODUCT_TEST_CASE ("Exp", "[VCTR][Expressions][exp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t))
{
    VCTR_TEST_DEFINES (10)

    // clang-format off
    const vctr::Vector exp  = filter << vctr::exp << srcA / ElementType (10);
    const vctr::Vector expU = filter << vctr::exp << srcUnaligned / ElementType (10);

    REQUIRE_THAT (exp,  vctr::EqualsTransformedBy<div10exp> (srcA).withEpsilon (0.00005));
    REQUIRE_THAT (expU, vctr::EqualsTransformedBy<div10exp> (srcUnaligned).withEpsilon (0.00005));
    // clang-format on
}

TEMPLATE_PRODUCT_TEST_CASE ("Exp complex", "[VCTR][Expressions][exp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (std::complex<float>, std::complex<double>))
{
    // Real parts in [-10, 10] keep e^real in a range where a relative epsilon is meaningful for float
    VCTR_TEST_DEFINES_IN_RANGE (-10, 10, 10)

    using RealType = vctr::RealType<ElementType>;

    // Compares real and imaginary parts separately, so that an error in only one of them is detected
    const auto requireEqualsStdExp = [] (const auto& result, const auto& src, double epsilon) {
        REQUIRE (result.size() == src.size());

        const vctr::Vector<RealType> realParts = vctr::real << result;
        const vctr::Vector<RealType> imagParts = vctr::imag << result;

        for (size_t i = 0; i < src.size(); ++i)
        {
            const auto expected = std::exp (ElementType (src[i]));

            INFO ("i = " << i << ", src = " << src[i] << ", result = " << result[i] << ", expected = " << expected);
            REQUIRE (realParts[i] == Catch::Approx (expected.real()).epsilon (epsilon).margin (epsilon));
            REQUIRE (imagParts[i] == Catch::Approx (expected.imag()).epsilon (epsilon).margin (epsilon));
        }
    };

    SECTION ("Runtime evaluation")
    {
        const vctr::Vector exp = filter << vctr::exp << srcA;
        const vctr::Vector expU = filter << vctr::exp << srcUnaligned;

        static_assert (std::same_as<typename decltype (exp)::value_type, ElementType>);

        requireEqualsStdExp (exp, srcA, 0.00005);
        requireEqualsStdExp (expU, srcUnaligned, 0.00005);
    }

    SECTION ("Chained with other expressions")
    {
        const vctr::Vector exp = filter << vctr::exp << srcC * ElementType (0.5);
        const vctr::Vector<ElementType> halfSrcC = srcC * ElementType (0.5);

        requireEqualsStdExp (exp, halfSrcC, 0.00005);
    }

    SECTION ("Compile time evaluation")
    {
        static constexpr vctr::Array expConstexpr = filter << vctr::exp << srcA;

        requireEqualsStdExp (expConstexpr, srcA, 0.0001);
    }
}
