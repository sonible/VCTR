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

// We chain an abs and ln operation in the test to feed the ln operation with positive values only.
// This function recreates that for the matcher
// clang-format off
template <vctr::is::signedNumber T>      auto absLn (T x) { return std::log (std::abs (x)); }
template <vctr::is::unsignedIntNumber T> auto absLn (T x) { return std::log (x); }
// clang-format on

TEMPLATE_PRODUCT_TEST_CASE ("Ln", "[ln]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t))
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector ln = filter << vctr::ln << vctr::abs << srcA;
    const vctr::Vector lnU = filter << vctr::ln << vctr::abs << srcUnaligned;

    REQUIRE_THAT (ln, vctr::EqualsTransformedBy<absLn> (srcA).withMargin (0.000001));
    REQUIRE_THAT (lnU, vctr::EqualsTransformedBy<absLn> (srcUnaligned).withMargin (0.000001));
}
