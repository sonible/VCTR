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

namespace
{
/** Records whether an instance was the source or the destination of a move, since the state of a moved-from object
    (e.g. an empty std::string) is not guaranteed by the standard.
*/
struct MoveTracker
{
    int value = 0;
    bool wasMovedFrom = false; // this instance was the source of a move
    bool wasMovedInto = false; // this instance was the destination of a move

    MoveTracker() = default;
    explicit MoveTracker (int v) : value (v) {}

    MoveTracker (const MoveTracker& other) : value (other.value) {}

    MoveTracker (MoveTracker&& other) noexcept : value (other.value)
    {
        wasMovedInto = true;
        other.wasMovedFrom = true;
        other.value = 0;
    }

    MoveTracker& operator= (const MoveTracker& other)
    {
        value = other.value;
        return *this;
    }

    MoveTracker& operator= (MoveTracker&& other) noexcept
    {
        value = other.value;
        wasMovedInto = true;
        other.wasMovedFrom = true;
        other.value = 0;
        return *this;
    }
};
} // namespace

TEST_CASE ("append", "[VCTR][ArrayMemberFunctions]")
{
    SECTION ("Append by copy")
    {
        vctr::Array<MoveTracker, 2> a { MoveTracker (1), MoveTracker (2) };
        vctr::Array<MoveTracker, 2> toAppend { MoveTracker (3), MoveTracker (4) };

        auto appended = a.append (toAppend);

        STATIC_REQUIRE (std::same_as<decltype (appended), vctr::Array<MoveTracker, 4>>);

        // Only the sources are checked: the wasMovedInto flags of the result depend on whether the compiler applies
        // NRVO to the returned Array, which is optional
        for (size_t i = 0; i < appended.size(); ++i)
            REQUIRE (appended[i].value == int (i) + 1);

        for (size_t i = 0; i < a.size(); ++i)
        {
            REQUIRE (a[i].value == int (i) + 1);
            REQUIRE_FALSE (a[i].wasMovedFrom);
            REQUIRE (toAppend[i].value == int (i) + 3);
            REQUIRE_FALSE (toAppend[i].wasMovedFrom);
        }
    }

    SECTION ("Append by move")
    {
        vctr::Array<MoveTracker, 2> a { MoveTracker (1), MoveTracker (2) };
        vctr::Array<MoveTracker, 2> toAppend { MoveTracker (3), MoveTracker (4) };

        auto appended = a.append (std::move (toAppend));

        STATIC_REQUIRE (std::same_as<decltype (appended), vctr::Array<MoveTracker, 4>>);

        for (size_t i = 0; i < appended.size(); ++i)
            REQUIRE (appended[i].value == int (i) + 1);

        // The elements of this Array are always copied, only the appended ones are moved
        REQUIRE_FALSE (a[0].wasMovedFrom);
        REQUIRE_FALSE (a[1].wasMovedFrom);

        // NOLINTBEGIN(bugprone-use-after-move): checking the moved-from state is the point of this test
        REQUIRE (toAppend[0].wasMovedFrom);
        REQUIRE (toAppend[1].wasMovedFrom);
        // NOLINTEND(bugprone-use-after-move)
    }

    SECTION ("Append by span")
    {
        vctr::Array a { 1.0f, 2.0f };
        vctr::Array b { 5.0f, 6.0f };

        vctr::Span s = b;
        auto appended = a.append (s);

        STATIC_REQUIRE (std::same_as<decltype (appended), vctr::Array<float, 4>>);
        REQUIRE_THAT (appended, vctr::Equals ({ 1.0f, 2.0f, 5.0f, 6.0f }));
    }

    SECTION ("constexpr append")
    {
        constexpr vctr::Array a { 1, 2 };
        constexpr vctr::Array b { 3, 4, 5 };
        constexpr auto appended = a.append (b);

        STATIC_REQUIRE (std::same_as<decltype (appended), const vctr::Array<int, 5>>);
        STATIC_REQUIRE (appended[0] == 1);
        STATIC_REQUIRE (appended[1] == 2);
        STATIC_REQUIRE (appended[2] == 3);
        STATIC_REQUIRE (appended[3] == 4);
        STATIC_REQUIRE (appended[4] == 5);
    }
}