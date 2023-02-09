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

TEMPLATE_PRODUCT_TEST_CASE ("Exp", "[exp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t))
{
    VCTR_TEST_DEFINES (10)

    // clang-format off
    const vctr::Vector exp  = filter << vctr::exp << srcA / ElementType (10);
    const vctr::Vector expU = filter << vctr::exp << srcUnaligned / ElementType (10);

    REQUIRE_THAT (exp,  vctr::EqualsTransformedBy<div10exp> (srcA).withEpsilon (0.00005));
    REQUIRE_THAT (expU, vctr::EqualsTransformedBy<div10exp> (srcUnaligned).withEpsilon (0.00005));
    // clang-format on
}
