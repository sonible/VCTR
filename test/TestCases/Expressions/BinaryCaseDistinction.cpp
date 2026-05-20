/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2026 - by sonible GmbH.

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

template <typename T, typename... U>
concept isAnyOf = (std::same_as<T, U> || ...);

template <isAnyOf<float, double, int32_t, int64_t> T>
T squareBelowOneCubeAbove (T x)
{
    if (x < T (1))
        return x * x;

    return x * x * x;
}

TEMPLATE_PRODUCT_TEST_CASE ("BinaryCaseDistinction", "[VCTR][Expressions][binaryCaseDistinction]", (DontUseAcceleration, VCTR_NATIVE_SIMD), (float, double, int32_t, int64_t) )
{
    VCTR_TEST_DEFINES (10)

    static constexpr auto threshold = ElementType (1);

    // square values below 1, cube them above
    const vctr::Vector cases = filter << vctr::binaryCaseDistinction<vctr::square, vctr::CompareOp::lessOrEqual, threshold, vctr::cube> << srcA;

    REQUIRE_THAT (cases, vctr::EqualsTransformedBy<squareBelowOneCubeAbove> (srcA).withEpsilon (0.000001));
}
