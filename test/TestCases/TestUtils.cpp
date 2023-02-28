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

TEMPLATE_TEST_CASE ("TestUtils", "[Vector][Constructor]", float, double)
{
    SECTION ("Equals with margin/epsilon")
    {
        const auto refValue = TestType (10);
        const auto testValue = TestType (11);

        const vctr::Vector<TestType> reference (10, refValue);

        vctr::Vector<TestType> v (10, refValue);
        REQUIRE_THAT (v, vctr::Equals (reference));

        v.fill (testValue);

        // margin
        auto marginPass = std::abs (testValue - refValue);
        auto marginFail = marginPass - std::numeric_limits<TestType>::epsilon();

        // baseline test
        for (auto n = 0; n < reference.size(); ++n)
        {
            CHECK (v[n] == Catch::Approx (reference[n]).margin (marginPass));
            CHECK_FALSE (v[n] == Catch::Approx (reference[n]).margin (marginFail));
        }

        REQUIRE (vctr::Equals (reference).withMargin (marginPass).match (v));
        REQUIRE_FALSE (vctr::Equals (reference).withMargin (marginFail).match (v));

        // epsilon
        auto epsilonPass = marginPass / refValue;
        auto epsilonFail = epsilonPass - std::numeric_limits<TestType>::epsilon();

        // baseline test
        for (auto n = 0; n < reference.size(); ++n)
        {
            CHECK (v[n] == Catch::Approx (reference[n]).epsilon (epsilonPass));
            CHECK_FALSE (v[n] == Catch::Approx (reference[n]).epsilon (epsilonFail));
        }

        REQUIRE (vctr::Equals (reference).withEpsilon (epsilonPass).match (v));
        REQUIRE_FALSE (vctr::Equals (reference).withEpsilon (epsilonFail).match (v));
    }
}
