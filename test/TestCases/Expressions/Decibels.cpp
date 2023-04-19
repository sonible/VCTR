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

// clang-format off
template <vctr::is::realFloatNumber T> auto magToDbFS    (T x) { return std::max (std::log10 (x) * T (20), T (-100)); }
template <vctr::is::realFloatNumber T> auto magToDbPower (T x) { return std::max (std::log10 (x) * T (10), T (-100)); }
// clang-format on

TEMPLATE_PRODUCT_TEST_CASE ("Decibels", "[Decibels]", (PlatformVectorOps, VCTR_NATIVE_SIMD), (float, double) )
{
    VCTR_TEST_DEFINES_WITH_TRAILING_ZERO_IN_RANGE (0, 2, 10)

    // clang-format off

    const vctr::Vector dBFS     = filter << vctr::magToDb<vctr::dBFS> << srcD;
    const vctr::Vector dBFSU    = filter << vctr::magToDb<vctr::dBFS> << srcUnaligned;
    const vctr::Vector dBPower  = filter << vctr::magToDb<vctr::dBPower> << srcD;
    const vctr::Vector dBPowerU = filter << vctr::magToDb<vctr::dBPower> << srcUnaligned;

    REQUIRE_THAT (dBFS,     vctr::EqualsTransformedBy<magToDbFS> (srcD).withEpsilon (0.000001));
    REQUIRE_THAT (dBFSU,    vctr::EqualsTransformedBy<magToDbFS> (srcUnaligned).withEpsilon (0.000001));
    REQUIRE_THAT (dBPower,  vctr::EqualsTransformedBy<magToDbPower> (srcD).withEpsilon (0.000001));
    REQUIRE_THAT (dBPowerU, vctr::EqualsTransformedBy<magToDbPower> (srcUnaligned).withEpsilon (0.000001));

    // The last source element is a magnitude of 0. This should result in the default minimum dB value which is -100
    REQUIRE (dBFS[9]     == ElementType (-100));
    REQUIRE (dBFSU[8]    == ElementType (-100));
    REQUIRE (dBPower[9]  == ElementType (-100));
    REQUIRE (dBPowerU[8] == ElementType (-100));

    // Transforming back to linear values should result in the original values
    const vctr::Vector magFS     = filter << vctr::dbToMag<vctr::dBFS> << dBFS;
    const vctr::Vector magFSU    = filter << vctr::dbToMag<vctr::dBFS> << dBFS.template subSpan<1>();
    const vctr::Vector magPower  = filter << vctr::dbToMag<vctr::dBPower> << dBPower;
    const vctr::Vector magPowerU = filter << vctr::dbToMag<vctr::dBPower> << dBPower.template subSpan<1>();

    REQUIRE_THAT (magFS,     vctr::Equals (srcD).withMargin (0.00001));
    REQUIRE_THAT (magFSU,    vctr::Equals (srcUnaligned).withMargin (0.00001));
    REQUIRE_THAT (magPower,  vctr::Equals (srcD).withMargin (0.00001));
    REQUIRE_THAT (magPowerU, vctr::Equals (srcUnaligned).withMargin (0.00001));

    // clang-format on
}
