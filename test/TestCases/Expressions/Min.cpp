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

TEMPLATE_PRODUCT_TEST_CASE ("Min", "[min]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, int64_t, uint32_t, uint64_t) )
{
    VCTR_TEST_DEFINES (10)
    constexpr auto srcOnlyNeg = UnitTestValues<ElementType>::template array<10, 0, -1000, -1>();

    const auto min = vctr::min << filter << srcA;
    const auto minU = vctr::min << filter << srcUnaligned;
    const auto minNeg = vctr::min << filter << srcOnlyNeg;

    REQUIRE (min == *std::min_element (srcA.begin(), srcA.end()));
    REQUIRE (minU == *std::min_element (srcUnaligned.begin(), srcUnaligned.end()));
    REQUIRE (minNeg == *std::min_element (srcOnlyNeg.begin(), srcOnlyNeg.end()));

    const auto minAbs = vctr::minAbs << filter << srcA;
    const auto minAbsU = vctr::minAbs << filter << srcUnaligned;
    const auto minAbsNeg = vctr::minAbs << filter << srcOnlyNeg;

    vctr::Vector srcAbs = vctr::abs << srcA;
    vctr::Vector srcAbsU = vctr::abs << srcUnaligned;
    vctr::Vector srcAbsNeg = vctr::abs << srcOnlyNeg;

    REQUIRE (minAbs == *std::min_element (srcAbs.begin(), srcAbs.end()));
    REQUIRE (minAbsU == *std::min_element (srcAbsU.begin(), srcAbsU.end()));
    REQUIRE (minAbsNeg == *std::min_element (srcAbsNeg.begin(), srcAbsNeg.end()));
}