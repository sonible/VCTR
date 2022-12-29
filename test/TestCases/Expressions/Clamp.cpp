/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2022 by sonible GmbH.

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

template <int bound, vctr::is::realNumber T>
auto clampLow (T x) { return std::max (x, T (bound)); }

template <int bound, vctr::is::realNumber T>
auto clampHigh (T x) { return std::min (x, T (bound)); }

template <int lo, int hi, vctr::is::realNumber T>
auto clamp (T x) { return std::clamp (x, T (lo), T (hi)); }

template <int bound, int factor, vctr::is::realNumber T>
auto clampLowMultiply (T x) { return std::max (x, T (bound)) * T (factor); }

TEMPLATE_PRODUCT_TEST_CASE ("ClampLow", "[clamp]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double, int32_t, uint32_t, int64_t, uint64_t) )
{
    VCTR_TEST_DEFINES (10)

    SECTION ("clampLowByConstant")
    {
        const vctr::Vector res = filter << vctr::clampLowByConstant<-2> << srcA;
        REQUIRE_THAT (res, vctr::EqualsTransformedBy<clampLow<-2>> (srcA));
    }

    SECTION ("clampLow")
    {
        const vctr::Vector res = filter << vctr::clampLow (ElementType (4)) << srcA;
        REQUIRE_THAT (res, vctr::EqualsTransformedBy<clampLow<4>> (srcA));
    }

    SECTION ("clampHighByConstant")
    {
        const vctr::Vector res = filter << vctr::clampHighByConstant<3> << srcA;
        REQUIRE_THAT (res, vctr::EqualsTransformedBy<clampHigh<3>> (srcA));
    }

    SECTION ("clampHigh")
    {
        const vctr::Vector res = filter << vctr::clampHigh (ElementType (-3)) << srcA;
        REQUIRE_THAT (res, vctr::EqualsTransformedBy<clampHigh<-3>> (srcA));
    }

    SECTION ("clampByConstant")
    {
        const vctr::Vector res = filter << vctr::clampByConstant<-6, 4> << srcA;
        REQUIRE_THAT (res, (vctr::EqualsTransformedBy<clamp<-6, 4>> (srcA)));
    }

    SECTION ("clamp")
    {
        const vctr::Vector res = filter << vctr::clamp (ElementType (-10), ElementType (2)) << srcA;
        REQUIRE_THAT (res, (vctr::EqualsTransformedBy<clamp<-10, 2>> (srcA)));
    }

    SECTION ("concatenation with multiplication")
    {
        const vctr::Vector res = filter << (vctr::clampLow (ElementType (4)) << srcA) * ElementType (2);
        REQUIRE_THAT (res, (vctr::EqualsTransformedBy<clampLowMultiply<4, 2>> (srcA)));
    }
}
