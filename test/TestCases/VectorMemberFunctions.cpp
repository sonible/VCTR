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

#include <list>

TEST_CASE ("Assignment operator", "[VectorMemberFunctions]")
{
    vctr::Vector<int> ints;

    // Assignment from an initializer list
    ints = { 1, 2, 3, 4 };
    REQUIRE_THAT (ints, vctr::Equals ({ 1, 2, 3, 4 }));

    // Assignment from a different container type
    std::array a { 5, 6, 7, 8, 9 };
    ints = a;
    REQUIRE_THAT (ints, vctr::Equals ({ 5, 6, 7, 8, 9 }));

    // Assignment from another Vector
    vctr::Vector v = { 10, 11, 12 };
    ints = v;
    REQUIRE_THAT (ints, vctr::Equals ({ 10, 11, 12 }));

    // Move-assignment from another Vector
    vctr::Vector v2 = { 13, 14, 15, 16 };
    ints = std::move (v2);
    REQUIRE_THAT (ints, vctr::Equals ({ 13, 14, 15, 16 }));

    // Assignment from a const reference
    vctr::Vector v3 = { 17, 18 };
    const auto& v3ref = v3;
    ints = v3ref;
    REQUIRE_THAT (ints, vctr::Equals ({ 17, 18 }));

    vctr::Vector<std::string> strings;

    // Move-assignment from a different container type
    std::array<std::string, 3> ma = { "foo", "bar", "baz" };
    strings = std::move (ma);
    REQUIRE_THAT (strings, vctr::Equals ({ "foo", "bar", "baz" }));

    // Move-assignment from the underlying container type
    std::vector<std::string> mv = { "I", "love", "sonible", "!" };
    strings = std::move (mv);
    REQUIRE_THAT (strings, vctr::Equals ({ "I", "love", "sonible", "!" }));
}

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

TEST_CASE ("push_back", "[VectorMemberFunctions]")
{
    vctr::Vector<std::string> v;

    const std::string byCopy = "By copy";
    std::string byMove = "By move";

    v.push_back ("In place string literal");
    v.push_back (byCopy);
    v.push_back (std::move (byMove));

    REQUIRE_THAT (v, vctr::Equals ({ "In place string literal", "By copy", "By move" }));
}

TEST_CASE ("pop_back", "[VectorMemberFunctions]")
{
    vctr::Vector<std::string> v { "one", "two", "three", "four" };

    auto last = v.pop_back();

    REQUIRE_THAT (last, Catch::Matchers::Equals ("four"));
    REQUIRE (v.size() == 3);
    REQUIRE_THAT (v, vctr::Equals ({ "one", "two", "three" }));
}

TEST_CASE ("emplace_back", "[VectorMemberFunctions]")
{
    vctr::Vector<std::pair<std::string, int>> v;

    v.emplace_back ("The answer", 42);

    REQUIRE (v.size() == 1);
    REQUIRE_THAT (v[0].first, Catch::Matchers::Equals ("The answer"));
    REQUIRE (v[0].second == 42);
}

TEST_CASE ("insert", "[VectorMemberFunctions]")
{
    vctr::Vector<std::string> v { "one", "two", "three", "four" };

    SECTION ("insert before iterator")
    {
        auto it1 = v.insert (v.begin() + 1, std::string ("move me"));
        REQUIRE_THAT (*it1, Catch::Matchers::Equals ("move me"));
        REQUIRE (std::distance (v.begin(), it1) == 1);

        auto it2 = v.insert (v.begin() + 3, "copy me");
        REQUIRE_THAT (*it2, Catch::Matchers::Equals ("copy me"));
        REQUIRE (std::distance (v.begin(), it2) == 3);

        auto it3 = v.insert (v.begin() + 5, 2, "two times");
        REQUIRE_THAT (*it3, Catch::Matchers::Equals ("two times"));
        REQUIRE (std::distance (v.begin(), it3) == 5);

        const vctr::Vector expected { "one", "move me", "two", "copy me", "three", "two times", "two times", "four" };
        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("insert at index")
    {
        auto it1 = v.insert (1, std::string ("move me"));
        REQUIRE_THAT (*it1, Catch::Matchers::Equals ("move me"));
        REQUIRE (std::distance (v.begin(), it1) == 1);

        auto it2 = v.insert (3, "copy me");
        REQUIRE_THAT (*it2, Catch::Matchers::Equals ("copy me"));
        REQUIRE (std::distance (v.begin(), it2) == 3);

        auto it3 = v.insert (5, 2, "two times");
        REQUIRE_THAT (*it3, Catch::Matchers::Equals ("two times"));
        REQUIRE (std::distance (v.begin(), it3) == 5);

        const vctr::Vector expected { "one", "move me", "two", "copy me", "three", "two times", "two times", "four" };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("insert range before iterator")
    {
        // Deliberately take a container with non-contiguous iterators and a different value type
        std::list<std::string_view> strings { "Foo", "Bar", "Baz" };

        auto it = v.insert (v.begin() + 2, strings.begin(), strings.end());
        REQUIRE_THAT (*it, Catch::Matchers::Equals ("Foo"));
        REQUIRE (std::distance (v.begin(), it) == 2);

        const vctr::Vector expected { "one", "two", "Foo", "Bar", "Baz", "three", "four" };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("insert range at position")
    {
        // Deliberately take a container with non-contiguous iterators and a different value type
        std::list<std::string_view> strings { "Foo", "Bar", "Baz" };

        auto it = v.insert (2, strings.begin(), strings.end());
        REQUIRE_THAT (*it, Catch::Matchers::Equals ("Foo"));
        REQUIRE (std::distance (v.begin(), it) == 2);

        const vctr::Vector expected { "one", "two", "Foo", "Bar", "Baz", "three", "four" };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("insert initializer list before iterator")
    {
        std::initializer_list<std::string> strings { "Foo", "Bar", "Baz" };

        auto it1 = v.insert (v.begin() + 2, strings);
        REQUIRE_THAT (*it1, Catch::Matchers::Equals ("Foo"));
        REQUIRE (std::distance (v.begin(), it1) == 2);

        auto it2 = v.insert (v.begin() + 4, { "in", "place" });
        REQUIRE_THAT (*it2, Catch::Matchers::Equals ("in"));
        REQUIRE (std::distance (v.begin(), it2) == 4);

        const vctr::Vector expected { "one", "two", "Foo", "Bar", "in", "place", "Baz", "three", "four" };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("insert initializer list at position")
    {
        std::initializer_list<std::string> strings { "Foo", "Bar", "Baz" };

        auto it1 = v.insert (2, strings);
        REQUIRE_THAT (*it1, Catch::Matchers::Equals ("Foo"));
        REQUIRE (std::distance (v.begin(), it1) == 2);

        auto it2 = v.insert (4, { "in", "place" });
        REQUIRE_THAT (*it2, Catch::Matchers::Equals ("in"));
        REQUIRE (std::distance (v.begin(), it2) == 4);

        const vctr::Vector expected { "one", "two", "Foo", "Bar", "in", "place", "Baz", "three", "four" };

        REQUIRE_THAT (v, vctr::Equals (expected));
    }

    SECTION ("insert Span and Array before iterator")
    {
        std::string s[] { "Copy 1", "Copy 2"};
        vctr::Span stringsToCopy (s, 2);

        std::string t[] { "Move Span 1", "Move Span 2"};
        vctr::Span stringsToMove (t, 2);

        vctr::Array<std::string, 2> strings { "From Array 1", "From Array 2" };

        auto it1 = v.insert (v.begin() + 2, stringsToCopy);
        REQUIRE_THAT (*it1, Catch::Matchers::Equals ("Copy 1"));
        REQUIRE (std::distance (v.begin(), it1) == 2);

        auto it2 = v.insert (v.begin() + 3, stringsToMove, true);
        REQUIRE_THAT (*it2, Catch::Matchers::Equals ("Move Span 1"));
        REQUIRE (std::distance (v.begin(), it2) == 3);

        auto it3 = v.insert (v.begin() + 4, std::move (strings));
        REQUIRE_THAT (*it3, Catch::Matchers::Equals ("From Array 1"));
        REQUIRE (std::distance (v.begin(), it3) == 4);

        const vctr::Vector expected { "one", "two", "Copy 1", "Move Span 1", "From Array 1", "From Array 2", "Move Span 2", "Copy 2", "three", "four" };

        REQUIRE_THAT (v, vctr::Equals (expected));

        REQUIRE_FALSE (s[0].empty());
        REQUIRE_FALSE (s[1].empty());
        REQUIRE (t[0].empty());
        REQUIRE (t[1].empty());
    }

    SECTION ("insert Span and Array at position")
    {
        std::string s[] { "Copy 1", "Copy 2"};
        vctr::Span stringsToCopy (s, 2);

        std::string t[] { "Move Span 1", "Move Span 2"};
        vctr::Span stringsToMove (t, 2);

        vctr::Array<std::string, 2> strings { "From Array 1", "From Array 2" };

        auto it1 = v.insert (2, stringsToCopy);
        REQUIRE_THAT (*it1, Catch::Matchers::Equals ("Copy 1"));
        REQUIRE (std::distance (v.begin(), it1) == 2);

        auto it2 = v.insert (3, stringsToMove, true);
        REQUIRE_THAT (*it2, Catch::Matchers::Equals ("Move Span 1"));
        REQUIRE (std::distance (v.begin(), it2) == 3);

        auto it3 = v.insert (4, std::move (strings));
        REQUIRE_THAT (*it3, Catch::Matchers::Equals ("From Array 1"));
        REQUIRE (std::distance (v.begin(), it3) == 4);

        const vctr::Vector expected { "one", "two", "Copy 1", "Move Span 1", "From Array 1", "From Array 2", "Move Span 2", "Copy 2", "three", "four" };

        REQUIRE_THAT (v, vctr::Equals (expected));

        REQUIRE_FALSE (s[0].empty());
        REQUIRE_FALSE (s[1].empty());
        REQUIRE (t[0].empty());
        REQUIRE (t[1].empty());
    }

    SECTION ("append")
    {
        const vctr::Vector toAppend { "five", "six", "seven" };
        v.append (toAppend);

        const vctr::Vector expected { "one", "two", "three", "four", "five", "six", "seven" };
        REQUIRE_THAT (v, vctr::Equals (expected));
    }
}

TEST_CASE ("swap", "[VectorMemberFunctions]")
{
    std::initializer_list<int> a { 1, 2, 3, 4 };
    std::initializer_list<int> b { 9, 8, 7, 6 };

    vctr::Vector vecA = a;
    vctr::Vector vecB = b;

    auto it1 = vecA.begin();
    auto it9 = vecB.begin();

    vecA.swap (vecB);

    // Iterators should still be valid after swapping
    REQUIRE (vecB.begin() == it1);
    REQUIRE (vecA.begin() == it9);

    REQUIRE_THAT (a, vctr::Equals (vecB));
    REQUIRE_THAT (b, vctr::Equals (vecA));
}
