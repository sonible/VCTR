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

TEMPLATE_PRODUCT_TEST_CASE ("Abs", "[abs]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, int64_t, std::complex<float>, std::complex<double>) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector abs = filter << vctr::abs << srcA;
    const vctr::Vector absU = filter << vctr::abs << srcUnaligned;

    // std::is_arithmetic is not true for complex values, so this checks if the return type is always real valued, even for
    // complex input
    static_assert (std::is_arithmetic_v<typename decltype (abs)::value_type>);
    static_assert (std::is_arithmetic_v<typename decltype (absU)::value_type>);

    /*  TODO: Re-enable tests after resolving compilation issue on linux-clang
        REQUIRE_THAT (abs, vctr::EqualsTransformedBy<std::abs> (srcA).withEpsilon());
        REQUIRE_THAT (absU, vctr::EqualsTransformedBy<std::abs> (srcUnaligned).withEpsilon());
    */
}
