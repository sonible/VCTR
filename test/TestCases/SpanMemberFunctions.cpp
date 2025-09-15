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

TEST_CASE ("Assignment operator", "[VCTR][SpanMemberFunctions]")
{
    std::array<int, 3> intStorage { 0 };
    vctr::Span ints (intStorage);

    // Assignment from an initializer list
    ints = { 1, 2, 3 };
    REQUIRE_THAT (ints, vctr::Equals ({ 1, 2, 3 }));

    // Assignment from a Vector
    vctr::Vector a = { 7, 8, 9 };
    ints = a;
    REQUIRE_THAT (ints, vctr::Equals ({ 7, 8, 9 }));

    std::array<std::string, 3> stringStorage;
    vctr::Span strings (stringStorage);

    // Move-assignment from a different container type
    vctr::Vector<std::string> mv = { "I", "love", "sonible" };
    strings = std::move (mv);
    REQUIRE_THAT (strings, vctr::Equals ({ "I", "love", "sonible" }));
}

TEST_CASE ("castTo", "[VCTR][SpanMemberFunctions]")
{
    SECTION ("Static extent")
    {
        constexpr vctr::Array<int32_t, 32> a { 1 << 16 | 1 };

        auto asInt16Fixed = vctr::Span (a).castTo<const int16_t>();
        auto asInt64Fixed = vctr::Span (a).castTo<const int64_t>();

        STATIC_REQUIRE (asInt16Fixed.size() == 64);
        STATIC_REQUIRE (asInt64Fixed.size() == 16);

        REQUIRE (asInt16Fixed.allElementsEqual (1));
        REQUIRE (asInt64Fixed.allElementsEqual (1ll << 48  |  1ll << 32 | 1ll << 16 | 1ll));
    }

    SECTION ("Dynamic extent")
    {
        vctr::Vector<float> f (8);
        vctr::Vector<std::complex<float>> cf (3);

        auto asCplx = vctr::Span (f).castTo<std::complex<float>>();

        REQUIRE (asCplx.size() == 4);
        asCplx.assign ({ { 0.1f, 1.2f }, { 2.3f, 3.4f }, { 4.5f, 5.6f }, { 6.7f, 7.8f } });
        REQUIRE_THAT (f, vctr::Equals ( { 0.1f, 1.2f, 2.3f, 3.4f, 4.5f, 5.6f, 6.7f, 7.8f }));

        auto asFloat = vctr::Span (cf).castTo<float>();

        REQUIRE (asFloat.size() == 6);
        asFloat.assign ({ -0.1f, -1.2f, -2.3f, -3.4f, -4.5f, -5.6f });
        REQUIRE_THAT (cf, vctr::Equals ( { std::complex (-0.1f, -1.2f), std::complex (-2.3f, -3.4f), std::complex (-4.5f, -5.6f) }));
    }
}