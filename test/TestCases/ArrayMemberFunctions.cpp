/*
  ==============================================================================
    DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.

    Copyright 2023 by sonible GmbH.

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

TEST_CASE ("Assignment operator", "[VCTR][ArrayMemberFunctions]")
{
    vctr::Array<int, 3> ints;

    // Assignment from an initializer list
    ints = { 1, 2, 3 };
    REQUIRE_THAT (ints, vctr::Equals ({ 1, 2, 3 }));

    // Assignment from a different container type
    std::vector v { 4, 5, 6 };
    ints = v;
    REQUIRE_THAT (ints, vctr::Equals ({ 4, 5, 6 }));

    // Assignment from another Array
    vctr::Array a = { 7, 8, 9 };
    ints = a;
    REQUIRE_THAT (ints, vctr::Equals ({ 7, 8, 9 }));

    // Move-assignment from another Array
    vctr::Array a2 = { 10, 11, 12 };
    ints = std::move (a2);
    REQUIRE_THAT (ints, vctr::Equals ({ 10, 11, 12 }));

    vctr::Array a3 = { 13, 14, 15 };
    const auto& a3ref = a3;
    ints = a3ref;
    REQUIRE_THAT (ints, vctr::Equals ({ 13, 14, 15 }));

    vctr::Array<std::string, 3> strings;

    // Move-assignment from the underlying container type
    std::array<std::string, 3> ma = { "foo", "bar", "baz" };
    strings = std::move (ma);
    REQUIRE_THAT (strings, vctr::Equals ({ "foo", "bar", "baz" }));

    // Move-assignment from a different container type
    vctr::Vector<std::string> mv = { "I", "love", "sonible" };
    strings = std::move (mv);
    REQUIRE_THAT (strings, vctr::Equals ({ "I", "love", "sonible" }));
}

TEST_CASE ("append", "[VCTR][ArrayMemberFunctions]")
{
    SECTION ("append by copy")
    {
        vctr::Array<std::string, 2> a { "one", "two" };
        vctr::Array<std::string, 2> toAppend { "three", "four" };

        auto appended = a.append (toAppend);

        REQUIRE (std::is_same_v<std::remove_cvref_t<decltype (appended)>, vctr::Array<std::string, 4>>);
        REQUIRE_THAT (appended, vctr::Equals ({ "one", "two", "three", "four" }));
        REQUIRE_FALSE (toAppend[0].empty());
        REQUIRE_FALSE (toAppend[1].empty());
    }

    SECTION ("append by move from Span")
    {
        vctr::Array<std::string, 2> a { "one", "two" };

        std::string s[] { "move 1", "move 2" };
        vctr::Span spanToMove (s, 2);

        auto appended = a.append (spanToMove, true);

        REQUIRE (std::is_same_v<std::remove_cvref_t<decltype (appended)>, vctr::Array<std::string, 4>>);
        REQUIRE_THAT (appended, vctr::Equals ({ "one", "two", "move 1", "move 2" }));
        REQUIRE (s[0].empty());
        REQUIRE (s[1].empty());
    }

    SECTION ("constexpr append")
    {
        constexpr vctr::Array<int, 2> a { 1, 2 };
        constexpr vctr::Array<int, 3> b { 3, 4, 5 };
        constexpr auto appended = a.append (b);

        STATIC_REQUIRE (std::is_same_v<std::remove_cvref_t<decltype (appended)>, vctr::Array<int, 5>>);
        STATIC_REQUIRE (appended[0] == 1);
        STATIC_REQUIRE (appended[1] == 2);
        STATIC_REQUIRE (appended[2] == 3);
        STATIC_REQUIRE (appended[3] == 4);
        STATIC_REQUIRE (appended[4] == 5);
    }
}