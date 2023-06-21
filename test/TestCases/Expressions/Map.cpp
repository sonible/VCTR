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

template <int srcMin, int srcMax, int dstMin, int dstMax, vctr::is::realNumber T>
T map (T val)
{
    constexpr auto tMin = std::numeric_limits<T>::lowest();
    constexpr auto tMax = std::numeric_limits<T>::max();

    static_assert (srcMin >= tMin && srcMin <= tMax);
    static_assert (srcMax >= tMin && srcMax <= tMax);
    static_assert (dstMin >= tMin && dstMin <= tMax);
    static_assert (dstMax >= tMin && dstMax <= tMax);

    val -= T (srcMin);
    val *= T (dstMax - dstMin);
    val /= T (srcMax - srcMin);
    val += T (dstMin);
    return val;
}

TEMPLATE_PRODUCT_TEST_CASE ("Map", "[expressions]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double))
{
    constexpr int srcValueMin = -52;
    constexpr int srcValueMax = 4000;

    constexpr int dstValueMin = -32768;
    constexpr int dstValueMax = -2;

    VCTR_TEST_DEFINES_IN_RANGE (srcValueMin, srcValueMax, 10)

    vctr::Vector mapped1 = filter << vctr::map (ElementType (srcValueMin), ElementType (srcValueMax), ElementType (dstValueMin), ElementType (dstValueMax)) << srcA;
    vctr::Vector mapped2 = filter << vctr::map (vctr::Range<ElementType> (srcValueMin, srcValueMax), vctr::Range<ElementType> (dstValueMin, dstValueMax)) << srcA;

    REQUIRE_THAT (mapped1, vctr::Equals (mapped2));
    REQUIRE (mapped1.min() >= ElementType (dstValueMin));
    REQUIRE (mapped1.max() <= ElementType (dstValueMax));

    REQUIRE_THAT (mapped1, (vctr::EqualsTransformedBy<map<srcValueMin, srcValueMax, dstValueMin, dstValueMax>> (srcA).withEpsilon (0.0001)));
}

TEMPLATE_PRODUCT_TEST_CASE ("MapFrom0To1", "[expressions]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double))
{
    constexpr int srcValueMin = 0;
    constexpr int srcValueMax = 1;

    constexpr int dstValueMin = -32768;
    constexpr int dstValueMax = -2;

    VCTR_TEST_DEFINES_IN_RANGE (srcValueMin, srcValueMax, 10)

    vctr::Vector mapped1 = filter << vctr::mapFrom0To1 (ElementType (dstValueMin), ElementType (dstValueMax)) << srcA;
    vctr::Vector mapped2 = filter << vctr::mapFrom0To1 (vctr::Range<ElementType> (dstValueMin, dstValueMax)) << srcA;

    REQUIRE_THAT (mapped1, vctr::Equals (mapped2));
    REQUIRE (mapped1.min() >= ElementType (dstValueMin));
    REQUIRE (mapped1.max() <= ElementType (dstValueMax));

    REQUIRE_THAT (mapped1, (vctr::EqualsTransformedBy<map<srcValueMin, srcValueMax, dstValueMin, dstValueMax>> (srcA).withEpsilon (0.0001)));
}

TEMPLATE_PRODUCT_TEST_CASE ("MapTo0To1", "[expressions]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double))
{
    constexpr int srcValueMin = -52;
    constexpr int srcValueMax = 4000;

    constexpr int dstValueMin = 0;
    constexpr int dstValueMax = 1;

    VCTR_TEST_DEFINES_IN_RANGE (srcValueMin, srcValueMax, 10)

    vctr::Vector mapped1 = filter << vctr::mapTo0To1 (ElementType (srcValueMin), ElementType (srcValueMax)) << srcA;
    vctr::Vector mapped2 = filter << vctr::mapTo0To1 (vctr::Range<ElementType> (srcValueMin, srcValueMax)) << srcA;

    REQUIRE_THAT (mapped1, vctr::Equals (mapped2));
    REQUIRE (mapped1.min() >= ElementType (dstValueMin));
    REQUIRE (mapped1.max() <= ElementType (dstValueMax));

    REQUIRE_THAT (mapped1, (vctr::EqualsTransformedBy<map<srcValueMin, srcValueMax, dstValueMin, dstValueMax>> (srcA).withEpsilon (0.0001)));
}
