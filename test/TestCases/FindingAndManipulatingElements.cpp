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

TEST_CASE ("count")
{
    vctr::Array a { 1, 2, 3, 1, 2, 1, 1, 4 };

    REQUIRE (a.count (1) == 4);
    REQUIRE (a.count (2) == 2);
    REQUIRE (a.count (3) == 1);
    REQUIRE (a.count (4) == 1);
    REQUIRE (a.count (5) == 0);

    REQUIRE (a.countIf ([] (auto v) { return v < 3; }) == 6);
    REQUIRE (a.countIf ([] (auto v) { return v % 2 != 0; }) == 5);
}