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

TEST_CASE ("Assignment operator", "[SpanMemberFunctions]")
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
