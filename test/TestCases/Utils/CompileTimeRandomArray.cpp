/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2026- by sonible GmbH.

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

// The test build defines VCTR_RANDOM_SEED for all test cases, override it with a known value for this TU.
#undef VCTR_RANDOM_SEED
#define VCTR_RANDOM_SEED 4711u

#include <vctr_test_utils/vctr_test_common.h>

TEST_CASE ("Compile time random array seed", "[VCTR][Utils][random]")
{
    constexpr uint32_t expectedSeed = 4711;

    SECTION ("VCTR_RANDOM_SEED overrides the seed")
    {
        STATIC_REQUIRE (vctr::random::getSeed() == expectedSeed);
    }

    SECTION ("Uniform distribution uses the seed plus offset")
    {
        constexpr auto withoutOffset = vctr::random::makeArrayWithUniformDistribution<16> (-100.0f, 100.0f);
        constexpr auto withOffset = vctr::random::makeArrayWithUniformDistribution<16> (-100.0f, 100.0f, 3);

        STATIC_REQUIRE (withoutOffset == vctr::random::detail::makeArrayWithUniformDistribution<16> (-100.0f, 100.0f, expectedSeed));
        STATIC_REQUIRE (withOffset == vctr::random::detail::makeArrayWithUniformDistribution<16> (-100.0f, 100.0f, expectedSeed + 3));
        STATIC_REQUIRE (withoutOffset != withOffset);
    }

    SECTION ("Normal distribution uses the seed")
    {
        constexpr auto normal = vctr::random::makeArrayWithNormalDistribution<16> (0.0, 1.0);

        STATIC_REQUIRE (normal == vctr::random::detail::makeArrayWithNormalDistribution<16> (0.0, 1.0, expectedSeed));
    }
}
