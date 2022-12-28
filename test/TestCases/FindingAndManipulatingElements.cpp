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

TEST_CASE ("find, findIf, findReverse, findIfReverse", "[VctrBase][Finding and manipulating elements]")
{
    vctr::Array       a { 1, 2, 3, 4, 5, 4, 3, 2, 1, 0 };
    const vctr::Array b { 1, 2, 3, 4, 5, 4, 3, 2, 1, 0 };

    // Making sure that the right values are found
    REQUIRE (*a.find (0) == 0);
    REQUIRE (*a.findReverse (1) == 1);
    REQUIRE (*b.find (2) == 2);
    REQUIRE (*b.findReverse (5) == 5);

    // Making sure that the right elements are found in case of duplicates
    REQUIRE (std::distance (a.begin(), a.find (3)) == 2);
    REQUIRE (std::distance (b.begin(), b.find (4)) == 3);
    REQUIRE (std::distance (a.rbegin(), a.findReverse (3)) == 3);
    REQUIRE (std::distance (b.rbegin(), b.findReverse (4)) == 4);

    vctr::Vector<std::string>       c { "one", "two", "three", "four" };
    const vctr::Vector<std::string> d { "one", "two", "three", "four" };

    auto beginsWith_t = [] (const auto& s) { return s[0] == 't'; };

    REQUIRE (*c.findIf (beginsWith_t) == "two");
    REQUIRE (*d.findIf (beginsWith_t) == "two");
    REQUIRE (*c.findIfReverse (beginsWith_t) == "three");
    REQUIRE (*d.findIfReverse (beginsWith_t) == "three");
}

TEST_CASE ("count, countIf", "[VctrBase][Finding and manipulating elements]")
{
    vctr::Array a { 1, 2, 3, 1, 2, 1, 1, 4 };
    const vctr::Array b { 1, 2, 3, 1, 2, 1, 1, 4 };

    REQUIRE (a.count (1) == 4);
    REQUIRE (b.count (1) == 4);
    REQUIRE (a.count (2) == 2);
    REQUIRE (b.count (2) == 2);
    REQUIRE (a.count (3) == 1);
    REQUIRE (b.count (3) == 1);
    REQUIRE (a.count (4) == 1);
    REQUIRE (b.count (4) == 1);
    REQUIRE (a.count (5) == 0);
    REQUIRE (b.count (5) == 0);

    REQUIRE (a.countIf ([] (auto v) { return v < 3; }) == 6);
    REQUIRE (b.countIf ([] (auto v) { return v < 3; }) == 6);
    REQUIRE (a.countIf ([] (auto v) { return v % 2 != 0; }) == 5);
    REQUIRE (b.countIf ([] (auto v) { return v % 2 != 0; }) == 5);
}
