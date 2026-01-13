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

TEMPLATE_PRODUCT_TEST_CASE ("MultiplyAccumulate", "[VCTR][Expressions][multiplyAccumulate]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector x = filter << vctr::multiplyAccumulate (srcA, srcB, srcC);

    vctr::Array<ElementType, 10> xElementWise;
    for (size_t i = 0; i < 10; ++i)
        xElementWise[i] = (srcA[i] * srcB[i]) + srcC[i];

    REQUIRE_THAT (x, vctr::Equals (xElementWise).withEpsilon (0.0001));
}

TEMPLATE_PRODUCT_TEST_CASE ("MultiplySubtract", "[VCTR][Expressions][multiplyAccumulate]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double) )
{
    VCTR_TEST_DEFINES (10)

    const vctr::Vector x = filter << vctr::multiplySubtract (srcA, srcB, srcC);

    vctr::Array<ElementType, 10> xElementWise;
    for (size_t i = 0; i < 10; ++i)
        xElementWise[i] = srcC[i] - (srcA[i] * srcB[i]);

    REQUIRE_THAT (x, vctr::Equals (xElementWise).withEpsilon (0.0001));
}
