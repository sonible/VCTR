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
#include <catch2/generators/catch_generators.hpp>

namespace vctr::test
{

TEMPLATE_TEST_CASE ("Linspace (simple tests)", "[VCTR][Generators][Linspace]", int, float, double)
{
    SECTION ("Length 1")
    {
        const size_t num = 1;
        vctr::Vector<TestType> result (num);
        result.fillLinspace (TestType (1), TestType (10), false);

        REQUIRE_THAT (result, vctr::Equals (vctr::generators::Linspace::makeVector (TestType (1), TestType (10), num, false)));

        REQUIRE (result.size() == 1);
        REQUIRE (result[0] == TestType (1));
    }

    SECTION ("start = stop")
    {
        const size_t num = 10;
        const auto value = TestType (3);

        vctr::Vector<TestType> result (num);
        result.fillLinspace (value, value, true);

        REQUIRE_THAT (result, vctr::Equals (vctr::generators::Linspace::makeVector (value, value, num, true)));

        // value should be repeated, compare numpy's linspace:
        // >>> import numpy as np
        // >>> np.linspace(start=3.0,stop=3.0, num=10, endpoint=True)
        // array([3., 3., 3., 3., 3., 3., 3., 3., 3., 3.])
        REQUIRE (result.allElementsEqual (value));
    }

    SECTION ("Equivalence of fillArray and fillVector")
    {
        constexpr auto num = std::is_floating_point_v<TestType> ? 101 : 100; // we need an integer increment for int vectors, otherwise we can be fractional
        constexpr auto start = TestType (1);
        constexpr auto stop = TestType (100);
        constexpr auto includeEnd = true;

        auto vector = vctr::generators::Linspace::makeVector<TestType> (start, stop, num, includeEnd);
        constexpr auto array = vctr::generators::Linspace::makeArray<TestType, num> (start, stop, includeEnd); // make sure makeArray can be constexpr evaluated

        REQUIRE (vector.size() == array.size());

        REQUIRE_THAT (vector, vctr::Equals (array));
    }
}

TEMPLATE_TEST_CASE ("Linspace (positive and negative increments)", "[VCTR][Generators][Linspace]", int, float, double)
{
    // Generate a run with normal and one with flipped sign
    const auto pos = TestType (GENERATE (1, -1));
    const auto neg = -pos;

    SECTION (pos > 0 ? "Normal sign" : "Reverse sign")
    {
        SECTION ("includeEnd")
        {
            const auto start = pos * TestType (10.0f);
            const auto stop = TestType (0.0f);
            // clang-format off
                auto withEnd    = vctr::generators::Linspace::makeVector<TestType> (start, stop, 11, true);
                auto withoutEnd = vctr::generators::Linspace::makeVector<TestType> (start, stop, 10, false);
            // clang-format on

            REQUIRE (withEnd.back() == stop);
            REQUIRE (withoutEnd.back() == stop + pos * TestType (1));
            REQUIRE_THAT (withEnd.subSpan (0, 10), vctr::Equals (withoutEnd));
        }

        SECTION ("integer step reverse")
        {
            // compare numpy's linspace:
            // >>> import numpy as np
            // >>> np.linspace(start=3.0, stop=-3.0, num=7, endpoint=True)
            // array([ 3.,  2.,  1.,  0., -1., -2., -3.])
            vctr::Vector<TestType> reference = { neg * TestType (3),
                                                 neg * TestType (2),
                                                 neg * TestType (1),
                                                 pos * TestType (0),
                                                 pos * TestType (1),
                                                 pos * TestType (2),
                                                 pos * TestType (3) };

            const auto start = neg * TestType (3.0f);
            const auto stop = pos * TestType (3.0f);
            const size_t num = 7;
            const auto includeEnd = true;

            vctr::Vector<TestType> result (num);
            result.fillLinspace (start, stop, includeEnd);

            REQUIRE_THAT (result, vctr::Equals (vctr::generators::Linspace::makeVector (start, stop, num, includeEnd)));

            for (auto n = 0; n < num; ++n)
                REQUIRE (reference[n] == Approx (result[n]));
        }

        SECTION ("Overload without num")
        {
            vctr::Vector<TestType> reference = { neg * TestType (2), neg * TestType (1), neg * TestType (0), pos * TestType (1) };

            const auto start = neg * TestType (2);
            const auto stop = pos * TestType (1);
            const auto includeEnd = true;

            auto result1 = vctr::generators::Linspace::makeVector (start, stop, includeEnd);
            vctr::Vector<TestType> result2 (result1.size());
            result2.fillLinspace (start, stop, includeEnd);

            REQUIRE_THAT (result1, vctr::Equals (result2));

            REQUIRE (result1.size() == reference.size());

            for (auto n = 0; n < result1.size(); ++n)
                REQUIRE (reference[n] == Approx (result1[n]));
        }
    }
}

TEMPLATE_TEST_CASE ("Linspace (additional tests for floating-point numbers)", "[VCTR][Generators][Linspace]", float, double)
{
    for (auto sign : { TestType (1), TestType (-1) })
    {
        const auto pos = sign;
        const auto neg = -sign;

        SECTION (pos > 0 ? "Normal sign" : "Reverse sign")
        {
            SECTION ("fractional step reverse")
            {
                auto start = neg * TestType (4.0f);
                auto stop = pos * TestType (3.0f);
                auto num = 7;
                auto includeEnd = true;

                // compare numpy's linspace:
                // >>> np.linspace(start=4.0, stop=-3.0, num=7, endpoint=True)
                // array([ 4.,  2.83333333,  1.66666667,  0.5, -0.66666667, -1.83333333, -3.])
                vctr::Vector<TestType> reference = { neg * TestType (4.0),
                                                     neg * TestType (2.83333333),
                                                     neg * TestType (1.66666667),
                                                     neg * TestType (0.5),
                                                     pos * TestType (0.66666667),
                                                     pos * TestType (1.83333333),
                                                     pos * TestType (3.0) };

                vctr::Vector<TestType> result (num);
                result.fillLinspace (start, stop, includeEnd);

                REQUIRE_THAT (result, vctr::Equals (vctr::generators::Linspace::makeVector (start, stop, num, includeEnd)));

                for (auto n = 0; n < num; ++n)
                    REQUIRE (reference[n] == Approx (result[n]));
            }

            SECTION ("Precision")
            {
                TestType start = pos * TestType (0.73);
                TestType stop = pos * TestType (1);
                size_t num = 123456789;
                auto includeEnd = true;

                vctr::Vector<TestType> result (num);
                result.fillLinspace (start, stop, includeEnd);

                REQUIRE_THAT (result, vctr::Equals (vctr::generators::Linspace::makeVector (start, stop, num, includeEnd)));

                REQUIRE (stop == Approx (result.back()).margin (std::numeric_limits<TestType>::epsilon()));
            }
        }
    }
}
} // namespace vctr::test
