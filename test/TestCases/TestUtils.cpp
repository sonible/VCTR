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

#include <vctr_test_utils/vctr_catch_matchers.h>
#include <vctr_test_utils/vctr_test_common.h>

TEMPLATE_TEST_CASE ("TestUtils", "[VCTR][TestUtils][Equals]", float, double)
{
    constexpr vctr::Array<TestType, 10> reference (TestType (1));

    SECTION ("Equals with margin")
    {
        constexpr auto margin = TestType (1e-3);

        constexpr vctr::Array vWithin = reference + margin - std::numeric_limits<TestType>::epsilon();
        constexpr vctr::Array vOutside = reference + margin + std::numeric_limits<TestType>::epsilon();

        // baseline test
        for (auto n = 0; n < reference.size(); ++n)
        {
            CHECK_THAT (vWithin[n], Catch::Matchers::WithinAbs (reference[n], margin));
            CHECK_FALSE (Catch::Matchers::WithinAbs (reference[n], margin).match (vOutside[n]));
        }

        REQUIRE_THAT (vWithin, vctr::Equals (reference).withMargin (margin));
        REQUIRE_FALSE (vctr::Equals (reference).withMargin (margin).match (vOutside));
    }

    SECTION ("Equals with epsilon")
    {
        constexpr auto epsilon = TestType (0.1);

        constexpr vctr::Array vWithin = reference + reference * epsilon;
        constexpr vctr::Array vOutside = reference + reference * (epsilon * TestType (2));

        // baseline test
        for (auto n = 0; n < reference.size(); ++n)
        {
            CHECK_THAT (vWithin[n], Catch::Matchers::WithinRel (reference[n], epsilon));
            CHECK_FALSE (Catch::Matchers::WithinRel (reference[n], epsilon).match (vOutside[n]));
        }

        REQUIRE_THAT (vWithin, vctr::Equals (reference).withEpsilon (epsilon));
        REQUIRE_FALSE (vctr::Equals (reference).withEpsilon (epsilon).match (vOutside));
    }
}
