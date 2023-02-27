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
T multiplication (T a, T b) { return a * b; }

TEMPLATE_PRODUCT_TEST_CASE ("Multiply", "[multiply]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const auto c = srcC[0];

    const vctr::Vector product1 = filter << (c * srcA);
    const vctr::Vector product2 = filter << (srcA * c);
    const vctr::Vector product3 = filter << (srcA * srcB);
    const vctr::Vector product4 = filter << vctr::multiplyByConstant<-2> << srcA;

    REQUIRE_THAT (product1, vctr::EqualsTransformedBy<multiplication> (srcA, c).withEpsilon());
    REQUIRE_THAT (product2, vctr::EqualsTransformedBy<multiplication> (srcA, c).withEpsilon());
    REQUIRE_THAT (product3, vctr::EqualsTransformedBy<multiplication> (srcA, srcB).withEpsilon());
    REQUIRE_THAT (product4, vctr::EqualsTransformedBy<multiplication> (srcA, ElementType (-2)).withEpsilon());
}

TEMPLATE_PRODUCT_TEST_CASE ("Multiply in place", "[multiply]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const auto c = srcC[0];

    auto product1 = srcA;
    auto product2 = srcA;

    product1 *= srcB;
    product2 *= c;

    REQUIRE_THAT (product1, vctr::EqualsTransformedBy<multiplication> (srcA, srcB).withEpsilon());
    REQUIRE_THAT (product2, vctr::EqualsTransformedBy<multiplication> (srcA, c).withEpsilon());
}
