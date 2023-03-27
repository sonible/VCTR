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

template <std::floating_point T>
T getAngle (std::complex<T> src) { return std::arg (src); }

TEMPLATE_PRODUCT_TEST_CASE ("Angle", "[complex]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector angle = filter << vctr::angle << srcA;
    const vctr::Vector angleU = filter << vctr::angle << srcUnaligned;

    // std::is_arithmetic is not true for complex values, so this checks if the return type is always real valued, even for
    // complex input
    static_assert (std::is_arithmetic_v<typename decltype (angle)::value_type>);
    static_assert (std::is_arithmetic_v<typename decltype (angleU)::value_type>);

    REQUIRE_THAT (angle, vctr::EqualsTransformedBy<getAngle> (srcA).withEpsilon (0.000001));
    REQUIRE_THAT (angleU, vctr::EqualsTransformedBy<getAngle> (srcUnaligned).withEpsilon (0.000001));
}
