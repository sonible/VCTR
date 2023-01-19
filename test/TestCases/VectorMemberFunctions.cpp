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

TEST_CASE ("capacity, shrink_to_fit, reserve, clear", "[VectorMemberFunctions]")
{
    auto v = UnitTestValues<int32_t>::template vector<100, 0>();

    REQUIRE (v.capacity() >= 100);

    // TODO: shrink_to_fit() seems to cause problems on Linux (VCTR-85/#49)
    /*
    v.shrink_to_fit();
    // shrink_to_fit() is a non-binding request, so we cannot check == 100 here
    REQUIRE (v.capacity() >= 100);
    */

    v.push_back (42);
    REQUIRE (v.capacity() >= 100);

    v.reserve (200);
    REQUIRE (v.capacity() >= 200);

    v.clear();
    REQUIRE (v.capacity() >= 0);

    // TODO: shrink_to_fit() seems to cause problems on Linux (VCTR-85/#49)
    /*
    v.shrink_to_fit();
    // shrink_to_fit() is a non-binding request, so we cannot check == 0 here
    REQUIRE (v.capacity() >= 0);
    */
}

TEST_CASE ("erase", "[VectorMemberFunctions]")
{
    vctr::Vector v { 0, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5 };

    auto isOdd = [] (auto v) { return v % 2 == 1; };

    SECTION ("erase single element using an iterator")
    {
        auto ninth = v.begin() + 8;
        auto it = v.erase (ninth);
        REQUIRE (*it == 4);

        const vctr::Vector expected { 0, 1, 2, 3, 4, 5, 1, 2, 4, 5 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("erase single element using an index")
    {
        auto it = v.erase (6);
        REQUIRE (*it == 2);

        const vctr::Vector expected { 0, 1, 2, 3, 4, 5, 2, 3, 4, 5 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("erase a range of elements using iterators")
    {
        auto first = v.begin() + 3;
        auto last = v.begin() + 5;
        auto it = v.erase (first, last);
        REQUIRE (*it == 5);

        const vctr::Vector expected { 0, 1, 2, 5, 1, 2, 3, 4, 5 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("erase a range of elements using index and size")
    {
        auto it = v.erase (6, 4);
        REQUIRE (*it == 5);

        const vctr::Vector expected { 0, 1, 2, 3, 4, 5, 5 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("erase first occurrence of a value")
    {
        auto it = v.eraseFirstOccurrenceOf (2);
        REQUIRE (*it == 3);

        const vctr::Vector expected { 0, 1, 3, 4, 5, 1, 2, 3, 4, 5 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("erase first predicate match")
    {
        auto it = v.eraseFirstOccurrenceIf (isOdd);
        REQUIRE (*it == 2);

        const vctr::Vector expected { 0, 2, 3, 4, 5, 1, 2, 3, 4, 5 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("erase all occurrences of a value")
    {
        v.eraseAllOccurrencesOf (4);

        const vctr::Vector expected { 0, 1, 2, 3, 5, 1, 2, 3, 5 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("erase all predicate matches")
    {
        v.eraseAllOccurrencesIf (isOdd);

        const vctr::Vector expected { 0, 2, 4, 2, 4 };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }
}
