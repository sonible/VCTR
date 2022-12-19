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

TEMPLATE_PRODUCT_TEST_CASE ("Max", "[max]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, int64_t, uint32_t, uint64_t) )
{
    VCTR_TEST_DEFINES (10)

    const auto max = vctr::max << filter << srcA;
    const auto maxU = vctr::max << filter << srcUnaligned;

    REQUIRE (max == srcA.max());
    REQUIRE (max == *std::max_element (srcA.begin(), srcA.end()));
    REQUIRE (maxU == *std::max_element (srcUnaligned.begin(), srcUnaligned.end()));

    const auto maxAbs = vctr::maxAbs << filter << srcA;
    const auto maxAbsU = vctr::maxAbs << filter << srcUnaligned;

    vctr::Vector srcAbs = vctr::abs << srcA;
    vctr::Vector srcAbsU = vctr::abs << srcUnaligned;

    REQUIRE (maxAbs == srcA.maxAbs());
    REQUIRE (maxAbs == *std::max_element (srcAbs.begin(), srcAbs.end()));
    REQUIRE (maxAbsU == *std::max_element (srcAbsU.begin(), srcAbsU.end()));
}