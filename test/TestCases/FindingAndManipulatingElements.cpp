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

TEST_CASE ("find, findIf, findReverse, findIfReverse, contains, indexOf, indexIf", "[VctrBase][Finding and manipulating elements]")
{
    vctr::Array       a { 1, 2, 3, 4, 5, 4, 3, 2, 1, 0 };
    const vctr::Array b { 1, 2, 3, 4, 5, 4, 3, 2, 1, 0 };

    // Making sure that the right values are found
    REQUIRE (*a.find (0) == 0);
    REQUIRE (*a.findReverse (1) == 1);
    REQUIRE (*b.find (2) == 2);
    REQUIRE (*b.findReverse (5) == 5);

    REQUIRE (a.contains (2));
    REQUIRE (b.contains (0));
    REQUIRE_FALSE (a.contains (-1));

    auto aIt = a.begin();
    REQUIRE (a.contains (aIt));
    ++aIt;
    REQUIRE (a.contains (aIt));
    aIt = a.end();
    REQUIRE_FALSE (a.contains (aIt));

    // Making sure that the right elements are found in case of duplicates
    REQUIRE (std::distance (a.begin(), a.find (3)) == 2);
    REQUIRE (std::distance (b.begin(), b.find (4)) == 3);
    REQUIRE (a.indexOf (3).value() == 2);
    REQUIRE (b.indexOf (4).value() == 3);
    REQUIRE (a.indexOf (6) == std::nullopt);
    REQUIRE (b.indexOf (6) == std::nullopt);
    REQUIRE (std::distance (a.rbegin(), a.findReverse (3)) == 3);
    REQUIRE (std::distance (b.rbegin(), b.findReverse (4)) == 4);
    REQUIRE (a.indexOfReverse (3).value() == 6);
    REQUIRE (b.indexOfReverse (4).value() == 5);
    REQUIRE (a.indexOfReverse (6) == std::nullopt);
    REQUIRE (b.indexOfReverse (6) == std::nullopt);

    vctr::Vector<std::string>       c { "one", "two", "three", "four" };
    const vctr::Vector<std::string> d { "one", "two", "three", "four" };

    auto beginsWith_t = [] (const auto& s) { return s[0] == 't'; };
    auto beginsWith_a = [] (const auto& s) { return s[0] == 'a'; };

    REQUIRE (*c.findIf (beginsWith_t) == "two");
    REQUIRE (*d.findIf (beginsWith_t) == "two");
    REQUIRE (c.findIf (beginsWith_a) == c.end());
    REQUIRE (d.findIf (beginsWith_a) == d.end());
    REQUIRE (c.indexIf (beginsWith_t).value() == 1);
    REQUIRE (d.indexIf (beginsWith_t).value() == 1);
    REQUIRE (c.indexIf (beginsWith_a) == std::nullopt);
    REQUIRE (d.indexIf (beginsWith_a) == std::nullopt);
    REQUIRE (*c.findIfReverse (beginsWith_t) == "three");
    REQUIRE (*d.findIfReverse (beginsWith_t) == "three");
    REQUIRE (c.findIfReverse (beginsWith_a) == c.rend());
    REQUIRE (d.findIfReverse (beginsWith_a) == d.rend());
    REQUIRE (c.indexIfReverse (beginsWith_t).value() == 2);
    REQUIRE (d.indexIfReverse (beginsWith_t).value() == 2);
    REQUIRE (c.indexIfReverse (beginsWith_a) == std::nullopt);
    REQUIRE (d.indexIfReverse (beginsWith_a) == std::nullopt);
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

TEST_CASE ("all, any, allElementsEqual", "[VctrBase][Finding and manipulating elements]")
{
    const auto loremIpsum = UnitTestValues<std::string>::template array<10, 0>();

    REQUIRE       (loremIpsum.any ([] (const std::string& s) { return s.starts_with ('s'); }));
    REQUIRE_FALSE (loremIpsum.all ([] (const std::string& s) { return s.starts_with ('s'); }));
    REQUIRE_FALSE (loremIpsum.allElementsEqual());

    const vctr::Array sameValues = { -2, -2, -2, -2, -2 };

    REQUIRE (sameValues.all ([] (const auto& v) { return v < 0; }));
    REQUIRE (sameValues.allElementsEqual());
    REQUIRE (sameValues.allElementsEqual (-2));
    REQUIRE_FALSE (sameValues.allElementsEqual (3));
}
