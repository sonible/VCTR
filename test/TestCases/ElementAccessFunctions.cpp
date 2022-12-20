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

// VCTR allows using the JUCE jassert assertion macro in VCTR_ASSERT if available. Here we abuse it to bypass
// the assertion which would kill the process in a debug build when testing the out-of-range exceptions
#define jassert(e)

#include <vctr_test_utils/vctr_test_common.h>

TEMPLATE_TEST_CASE ("Element access functions", "[VctrBase][Element Access]", float, int64_t, std::string)
{
    auto v = UnitTestValues<TestType>::template vector<100, 0>();
    auto a = UnitTestValues<TestType>::template array<100, 0>();
    auto s = vctr::Span (v);

    const auto cv = UnitTestValues<TestType>::template vector<100, 0>();
    const auto ca = UnitTestValues<TestType>::template array<100, 0>();
    auto cs = vctr::Span (cv);

    auto vFirst = *v.data();
    auto vSecond = *(v.data() + 1);
    auto aFirst = *a.data();
    auto aSecond = *(a.data() + 1);
    auto cvFirst = *cv.data();
    auto cvSecond = *(cv.data() + 1);
    auto caFirst = *ca.data();
    auto caSecond = *(ca.data() + 1);

    // operator[]
    REQUIRE (v[0] == vFirst);
    REQUIRE (v[1] == vSecond);
    REQUIRE (a[0] == aFirst);
    REQUIRE (a[1] == aSecond);
    REQUIRE (s[0] == vFirst);
    REQUIRE (s[1] == vSecond);
    REQUIRE (cv[0] == cvFirst);
    REQUIRE (cv[1] == cvSecond);
    REQUIRE (ca[0] == caFirst);
    REQUIRE (ca[1] == caSecond);
    REQUIRE (cs[0] == cvFirst);
    REQUIRE (cs[1] == cvSecond);

    // at
    REQUIRE (v.at (0) == vFirst);
    REQUIRE (v.at (1) == vSecond);
    REQUIRE (a.at (0) == aFirst);
    REQUIRE (a.at (1) == aSecond);
    REQUIRE (cv.at (0) == cvFirst);
    REQUIRE (cv.at (1) == cvSecond);
    REQUIRE (ca.at (0) == caFirst);
    REQUIRE (ca.at (1) == caSecond);

    // Ensure that exceptions are thrown for at if the value is out of range
    REQUIRE_THROWS_AS (v.at (100), std::out_of_range);
    REQUIRE_THROWS_AS (a.at (100), std::out_of_range);
}

TEMPLATE_TEST_CASE ("Iterators", "[VctrBase][Iterators]", float, int64_t, std::string)
{
    constexpr auto numElements = 5;

    auto v = UnitTestValues<TestType>::template vector<numElements, 0>();
    auto a = UnitTestValues<TestType>::template array<numElements, 0>();
    auto s = vctr::Span (v);

    const auto cv = UnitTestValues<TestType>::template vector<numElements, 0>();
    const auto ca = UnitTestValues<TestType>::template array<numElements, 0>();
    auto cs = vctr::Span (cv);

    auto vIt = v.begin();
    auto aIt = a.begin();
    auto sIt = s.begin();
    auto cvIt = cv.begin();
    auto caIt = ca.begin();
    auto csIt = cs.begin();

    for (size_t i = 0; i < numElements; ++i, ++vIt, ++aIt, ++sIt, ++cvIt, ++caIt, ++csIt)
    {
        REQUIRE (*vIt == v[i]);
        REQUIRE (*aIt == a[i]);
        REQUIRE (*sIt == s[i]);
        REQUIRE (*cvIt == cv[i]);
        REQUIRE (*caIt == ca[i]);
        REQUIRE (*csIt == cs[i]);
    }

    REQUIRE (vIt == v.end());
    REQUIRE (aIt == a.end());
    REQUIRE (sIt == s.end());
    REQUIRE (cvIt == cv.end());
    REQUIRE (caIt == ca.end());
    REQUIRE (csIt == cs.end());

    auto vRIt = v.rbegin();
    auto aRIt = a.rbegin();
    auto sRIt = s.rbegin();
    auto cvRIt = cv.rbegin();
    auto caRIt = ca.rbegin();
    auto csRIt = cs.rbegin();

    for (size_t i = numElements; i > 0; --i, ++vRIt, ++aRIt, ++sRIt, ++cvRIt, ++caRIt, ++csRIt)
    {
        REQUIRE (*vRIt == v[i - 1]);
        REQUIRE (*aRIt == a[i - 1]);
        REQUIRE (*sRIt == s[i - 1]);
        REQUIRE (*cvRIt == cv[i - 1]);
        REQUIRE (*caRIt == ca[i - 1]);
        REQUIRE (*csRIt == cs[i - 1]);
    }

    REQUIRE (vRIt == v.rend());
    REQUIRE (aRIt == a.rend());
    REQUIRE (sRIt == s.rend());
    REQUIRE (cvRIt == cv.rend());
    REQUIRE (caRIt == ca.rend());
    REQUIRE (csRIt == cs.rend());
}
