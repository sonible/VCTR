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
T addition (T a, T b) { return a + b; }

TEMPLATE_PRODUCT_TEST_CASE ("SIMDFilter", "[VCTR][SIMD]", (AnySIMD), (float, double, int32_t, uint32_t) )
{
    VCTR_TEST_DEFINES (10)
    const vctr::Vector sum = vctr::assertSIMD << filter << (srcA + srcB);

    REQUIRE_THAT (sum, vctr::EqualsTransformedBy<addition> (srcA, srcB));
}
