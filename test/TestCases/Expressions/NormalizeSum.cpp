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

TEMPLATE_PRODUCT_TEST_CASE ("NormalizeSum", "[expressions]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double))
{
    VCTR_TEST_DEFINES (10)

    vctr::Vector normalizedSum = filter << vctr::normalizeSum << (srcA + ElementType (1));

    vctr::Vector srcAPlus1 = srcA + ElementType (1);

    const auto sum = vctr::sum << srcAPlus1;

    vctr::Vector dividedBySum = srcAPlus1 / sum;

    REQUIRE_THAT (normalizedSum, vctr::Equals (dividedBySum).withEpsilon (0.000001));
}